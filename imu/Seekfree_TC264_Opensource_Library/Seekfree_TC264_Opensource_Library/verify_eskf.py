"""
ESKF Python verification - decoupled architecture (proven best)
HeadingCF owns heading+bias independently
ESKF handles position+velocity+acc_bias (9 error states)
"""

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

GRAVITY = 9.80

# ============================================================
#  Quaternion / matrix helpers
# ============================================================

def quat_normalize(q):
    n = np.linalg.norm(q)
    return q / n if n > 1e-10 else np.array([1, 0, 0, 0], dtype=float)

def quat_mul(p, q):
    w0, x0, y0, z0 = p
    w1, x1, y1, z1 = q
    return np.array([
        w0*w1 - x0*x1 - y0*y1 - z0*z1,
        w0*x1 + x0*w1 + y0*z1 - z0*y1,
        w0*y1 - x0*z1 + y0*w1 + z0*x1,
        w0*z1 + x0*y1 - y0*x1 + z0*w1,
    ])

def quat_to_rot(q):
    w, x, y, z = q
    x2, y2, z2 = x*x, y*y, z*z
    xy, xz, yz = x*y, x*z, y*z
    wx, wy, wz = w*x, w*y, w*z
    return np.array([
        [1-2*(y2+z2), 2*(xy-wz),   2*(xz+wy)],
        [2*(xy+wz),   1-2*(x2+z2), 2*(yz-wx)],
        [2*(xz-wy),   2*(yz+wx),   1-2*(x2+y2)],
    ])

def quat_update(q, w, dt):
    half_dt = 0.5 * dt
    wx, wy, wz = w * half_dt
    nw = np.linalg.norm([wx, wy, wz])
    if nw > 1e-10:
        cos_nw = np.cos(nw)
        sinc = np.sin(nw) / nw
    else:
        cos_nw = 1.0
        sinc = 1.0
    dq = np.array([cos_nw, wx*sinc, wy*sinc, wz*sinc])
    return quat_normalize(quat_mul(q, dq))

def euler_to_quat(roll, pitch, yaw):
    cr, sr = np.cos(roll/2), np.sin(roll/2)
    cp, sp = np.cos(pitch/2), np.sin(pitch/2)
    cy, sy = np.cos(yaw/2), np.sin(yaw/2)
    return np.array([cr*cp*cy + sr*sp*sy, sr*cp*cy - cr*sp*sy,
                     cr*sp*cy + sr*cp*sy, cr*cp*sy - sr*sp*cy])

def quat_to_euler(q):
    w, x, y, z = q
    pitch = np.arcsin(np.clip(2*(w*y - z*x), -1, 1))
    roll  = np.arctan2(2*(w*x + y*z), 1 - 2*(x*x + y*y))
    yaw   = np.arctan2(2*(w*z + x*y), 1 - 2*(y*y + z*z))
    return roll, pitch, yaw


# ============================================================
#  Heading Complementary Filter (standalone, matches C ins_update_mag)
# ============================================================

class HeadingCF:
    """Standalone heading complementary filter.
    Owns heading and gyro z-bias completely.
    Uses body-frame mag directly (no NED rotation to avoid feedback loop).
    """
    def __init__(self, alpha=0.3, beta=0.01):
        self.yaw = 0.0
        self.gyro_bias_z = 0.0
        self.alpha = alpha    # heading correction gain (matches C: MAG_ALPHA)
        self.beta = beta      # bias correction gain (matches C: MAG_BETA)

    def predict(self, gyro_z, dt):
        """Integrate gyro to get heading."""
        self.yaw += (gyro_z - self.gyro_bias_z) * dt
        self.yaw = (self.yaw + np.pi) % (2*np.pi) - np.pi

    def update_mag(self, mag_body, mag_declination=-0.105):
        """Correct heading from magnetometer.
        Uses body-frame components directly (level body assumption).
        yaw_true = atan2(-mag_y, mag_x) + declination
        """
        yaw_mag = np.arctan2(-mag_body[1], mag_body[0]) + mag_declination
        innov = yaw_mag - self.yaw
        innov = (innov + np.pi) % (2*np.pi) - np.pi
        self.yaw += self.alpha * innov
        self.yaw = (self.yaw + np.pi) % (2*np.pi) - np.pi
        self.gyro_bias_z -= self.beta * innov

    def get_rotation(self):
        """Get 3x3 rotation matrix (body->NED) from current heading."""
        q = euler_to_quat(0, 0, self.yaw)
        return quat_to_rot(q)


# ============================================================
#  ESKF (position + velocity + acc_bias, 9 error states)
# ============================================================

class ESKF:
    """ESKF with position, velocity, and accelerometer bias.
    Heading is provided externally by HeadingCF.
    Error state: [dp(3), dv(3), dba(3)] = 9 dimensions
    """
    def __init__(self):
        self.pos = np.zeros(3)
        self.vel = np.zeros(3)
        self.acc_bias = np.zeros(3)

        self.P = np.eye(9) * 0.1
        self.P[0:3, 0:3] = np.eye(3) * 1.0
        self.P[3:6, 3:6] = np.eye(3) * 1.0
        self.P[6:9, 6:9] = np.eye(3) * 0.1

        self.Qc_pos = 0.001
        self.Qc_vel = 0.01
        self.Qc_ba  = 0.001
        self.Rc = np.diag([1.0, 1.0, 2.0])

        self.pos_history = []
        self.pos_std_history = []
        self.bias_history = []

    def predict(self, acc_body, R_body2ned, dt):
        """Predict step using body-frame acc and CF-provided rotation."""
        acc_corrected = acc_body - self.acc_bias
        acc_ned = R_body2ned @ acc_corrected

        self.pos += self.vel*dt + 0.5*acc_ned*dt*dt
        self.vel += acc_ned * dt

        Fd = np.eye(9)
        Fd[0:3, 3:6] = np.eye(3) * dt
        Fd[3:6, 6:9] = -R_body2ned * dt

        Qd = np.zeros((9, 9))
        Qd[0:3, 0:3] = np.eye(3) * self.Qc_pos * dt
        Qd[3:6, 3:6] = np.eye(3) * self.Qc_vel * dt
        Qd[6:9, 6:9] = np.eye(3) * self.Qc_ba * dt

        self.P = Fd @ self.P @ Fd.T + Qd
        self._record()

    def update_gps(self, gps_pos):
        """GPS update - affects position, velocity, and acc_bias."""
        innov = gps_pos - self.pos
        S = self.P[0:3, 0:3] + self.Rc

        det = (S[0,0]*(S[1,1]*S[2,2]-S[1,2]*S[2,1])
             - S[0,1]*(S[1,0]*S[2,2]-S[1,2]*S[2,0])
             + S[0,2]*(S[1,0]*S[2,1]-S[1,1]*S[2,0]))
        if abs(det) < 1e-20:
            return
        inv_det = 1.0 / det
        S_inv = np.array([
            [(S[1,1]*S[2,2]-S[1,2]*S[2,1]), -(S[0,1]*S[2,2]-S[0,2]*S[2,1]),  (S[0,1]*S[1,2]-S[0,2]*S[1,1])],
            [-(S[1,0]*S[2,2]-S[1,2]*S[2,0]), (S[0,0]*S[2,2]-S[0,2]*S[2,0]), -(S[0,0]*S[1,2]-S[0,2]*S[1,0])],
            [(S[1,0]*S[2,1]-S[1,1]*S[2,0]), -(S[0,0]*S[2,1]-S[0,1]*S[2,0]),  (S[0,0]*S[1,1]-S[0,1]*S[1,0])],
        ]) * inv_det

        K = self.P[:, 0:3] @ S_inv
        dx = K @ innov

        self.pos += dx[0:3]
        self.vel += dx[3:6]
        self.acc_bias += dx[6:9]

        H = np.zeros((3, 9))
        H[0:3, 0:3] = np.eye(3)
        I_KH = np.eye(9) - K @ H
        self.P = I_KH @ self.P @ I_KH.T + K @ self.Rc @ K.T
        self._record()

    def set_position(self, x, y, z):
        self.pos = np.array([x, y, z], dtype=float)

    def _record(self):
        self.pos_history.append(self.pos.copy())
        self.bias_history.append(self.acc_bias.copy())
        self.pos_std_history.append(np.sqrt(max(0, self.P[0,0]+self.P[1,1]+self.P[2,2])))


# ============================================================
#  Trajectory generators
# ============================================================

def gen_circle(duration, dt, radius=10.0, speed=2.0):
    omega = speed / radius
    t = np.arange(0, duration, dt)
    yaw = omega * t
    x = radius * np.sin(yaw)
    y = radius * (1 - np.cos(yaw))
    vx = speed * np.cos(yaw)
    vy = speed * np.sin(yaw)
    ax = -speed * omega * np.sin(yaw)
    ay = speed * omega * np.cos(yaw)
    return t, x, y, vx, vy, ax, ay, np.full_like(t, omega), yaw

def gen_straight(duration, dt, speed=2.0):
    t = np.arange(0, duration, dt)
    n = len(t)
    return t, speed*t, np.zeros(n), np.full(n, speed), np.zeros(n), np.zeros(n), np.zeros(n), np.zeros(n), np.zeros(n)

def gen_rectangle(duration, dt, length=20.0, width=10.0, speed=2.0, R=2.0):
    legs = [(length, 0), (0, width), (-length, 0), (0, -width)]
    xs, ys, vxs, vys, axs, ays, gzs, yaws = [], [], [], [], [], [], [], []
    xc, yc, yawc = 0.0, 0.0, 0.0

    for dx, dy in legs:
        tyaw = np.arctan2(dx, dy)
        dyaw = (tyaw - yawc + np.pi) % (2*np.pi) - np.pi
        arc_dur = abs(dyaw) * R / speed
        omega = dyaw / arc_dur if arc_dur > 0 else 0
        n_arc = max(1, int(arc_dur / dt))
        for i in range(n_arc):
            yi = yawc + dyaw * (i+1) / n_arc
            xc += speed * np.cos(yi) * dt
            yc += speed * np.sin(yi) * dt
            xs.append(xc); ys.append(yc)
            vxs.append(speed*np.cos(yi)); vys.append(speed*np.sin(yi))
            axs.append(0); ays.append(0); gzs.append(omega); yaws.append(yi)
        yawc = tyaw

        dist = np.sqrt(dx**2 + dy**2)
        n_seg = max(1, int(dist / speed / dt))
        for _ in range(n_seg):
            xc += speed*np.cos(yawc)*dt; yc += speed*np.sin(yawc)*dt
            xs.append(xc); ys.append(yc)
            vxs.append(speed*np.cos(yawc)); vys.append(speed*np.sin(yawc))
            axs.append(0); ays.append(0); gzs.append(0); yaws.append(yawc)

    n = min(len(xs), int(duration/dt))
    t = np.arange(n) * dt
    return t, np.array(xs[:n]), np.array(ys[:n]), np.array(vxs[:n]), np.array(vys[:n]), \
           np.array(axs[:n]), np.array(ays[:n]), np.array(gzs[:n]), np.array(yaws[:n])


# ============================================================
#  Sensor simulation
# ============================================================

def sim_imu(ax_ned, ay_ned, gz, yaw,
            acc_bias=np.array([0.1, -0.08, 0.05]),
            gyro_bias=0.02, acc_noise=0.05, gyro_noise=0.01,
            mag_noise=0.01, mag_declination=-0.105):
    """Simulate calibrated IMU + magnetometer output."""
    n = len(ax_ned)
    acc = np.zeros((n, 3))
    gyro = np.zeros((n, 3))
    mag = np.zeros((n, 3))
    B_mag = 0.5
    B_down = 0.1
    B_true_ned = np.array([B_mag * np.cos(mag_declination),
                           B_mag * np.sin(mag_declination),
                           B_down])
    for i in range(n):
        c, s = np.cos(yaw[i]), np.sin(yaw[i])
        acc[i, 0] = c*ax_ned[i] + s*ay_ned[i]
        acc[i, 1] = -s*ax_ned[i] + c*ay_ned[i]
        gyro[i, 2] = gz[i]
        R_ned2body = np.array([[c, s, 0], [-s, c, 0], [0, 0, 1]])
        mag[i] = R_ned2body @ B_true_ned
    for i in range(n):
        acc[i] += acc_bias + np.random.randn(3) * acc_noise
        gyro[i, 2] += gyro_bias + np.random.randn() * gyro_noise
        mag[i] += np.random.randn(3) * mag_noise
    return acc, gyro, mag

def sim_gps(true_x, true_y, noise_std=1.5, gps_dt=0.1, imu_dt=0.01):
    ratio = int(gps_dt / imu_dt)
    idx = list(range(0, len(true_x), ratio))
    gx = true_x[idx] + np.random.randn(len(idx)) * noise_std
    gy = true_y[idx] + np.random.randn(len(idx)) * noise_std
    return gx, gy, idx


# ============================================================
#  Run simulation
# ============================================================

def run_sim(name, tx, ty, tyaw, acc, gyro, gx, gy, gi, mag=None,
            imu_dt=0.01, true_acc_bias=None, true_gyro_bias=None,
            gps_loss_ranges=None, use_mag=True):
    cf = HeadingCF(alpha=0.3, beta=0.01)
    eskf = ESKF()
    eskf.set_position(0, 0, 0)
    n = len(tx)
    gp = 0

    yaw_history = []
    cf_bias_history = []

    for i in range(n):
        # CF heading prediction
        cf.predict(gyro[i, 2], imu_dt)

        # Magnetometer heading update (every 10 steps = 10Hz)
        if use_mag and mag is not None and i % 10 == 0:
            cf.update_mag(mag[i], mag_declination=-0.105)

        # Get rotation from CF
        R = cf.get_rotation()

        # ESKF predict
        eskf.predict(acc[i], R, imu_dt)

        # GPS update
        if gp < len(gi) and i == gi[gp]:
            skip = False
            if gps_loss_ranges:
                for s, e in gps_loss_ranges:
                    if s <= i <= e: skip = True
            if not skip:
                eskf.update_gps(np.array([gx[gp], gy[gp], 0.0]))
            gp += 1

        yaw_history.append(cf.yaw)
        cf_bias_history.append(cf.gyro_bias_z)

    est = np.array(eskf.pos_history)
    pstd = np.array(eskf.pos_std_history)
    yaw_hist = np.array(yaw_history)
    cf_bias = np.array(cf_bias_history)
    ba_hist = np.array(eskf.bias_history)

    # Metrics
    nm = min(len(est), len(tx))
    pe = np.sqrt((est[:nm,0]-tx[:nm])**2 + (est[:nm,1]-ty[:nm])**2)
    ge = np.sqrt((gx - tx[gi])**2 + (gy - ty[gi])**2)
    ye = np.abs(yaw_hist[:nm] - tyaw[:nm])
    ye = np.minimum(ye, 2*np.pi - ye)

    print(f"\n{'='*55}")
    print(f"  {name}")
    print(f"{'='*55}")
    print(f"  ESKF pos err:  mean={np.mean(pe):.3f}m  max={np.max(pe):.3f}m")
    print(f"  GPS  pos err:  mean={np.mean(ge):.3f}m  max={np.max(ge):.3f}m")
    print(f"  Heading err:   mean={np.degrees(np.mean(ye)):.1f}deg  max={np.degrees(np.max(ye)):.1f}deg")
    print(f"  pos_std final: {pstd[-1]:.3f}m")
    print(f"  CF gyro bias:  est={cf_bias[-1]:.4f}  true={true_gyro_bias:.4f}")
    if true_acc_bias is not None:
        be = np.linalg.norm(ba_hist[-1] - true_acc_bias)
        print(f"  Acc bias err:  {be:.4f}  est=[{ba_hist[-1,0]:.3f},{ba_hist[-1,1]:.3f},{ba_hist[-1,2]:.3f}]")

    return est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist


# ============================================================
#  Plotting
# ============================================================

def plot_result(name, tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, true_gb, ba_hist, true_ba):
    fig, axes = plt.subplots(2, 3, figsize=(18, 10))
    fig.suptitle(f'ESKF (9-state PV+Ba) + HeadingCF: {name}', fontsize=14)

    ax = axes[0,0]
    ax.plot(tx, ty, 'g-', lw=2, label='True', alpha=.7)
    ax.plot(est[:,0], est[:,1], 'b-', lw=1, label='ESKF', alpha=.8)
    ax.plot(gx, gy, 'r.', ms=3, label='GPS', alpha=.5)
    ax.set_xlabel('North (m)'); ax.set_ylabel('East (m)')
    ax.legend(); ax.set_aspect('equal'); ax.grid(True); ax.set_title('Trajectory')

    ax = axes[0,1]
    ax.plot(np.arange(len(pe))*0.01, pe, 'b-', label='ESKF')
    ax.plot(np.arange(len(ge))*0.1, ge, 'r-', label='GPS')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Error (m)')
    ax.legend(); ax.grid(True); ax.set_title('Position Error')

    ax = axes[0,2]
    ax.plot(np.arange(len(ye))*0.01, np.degrees(ye), 'b-')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Heading Error (deg)')
    ax.grid(True); ax.set_title('Heading Error')

    ax = axes[1,0]
    ax.plot(np.arange(len(pstd))*0.01, pstd, 'b-')
    ax.axhline(1, color='r', ls='--', alpha=.5)
    ax.set_xlabel('Time (s)'); ax.set_ylabel('pos_std (m)')
    ax.grid(True); ax.set_title('Covariance Convergence')

    ax = axes[1,1]
    t_b = np.arange(len(ba_hist))*0.01
    for i, lbl in enumerate(['bx','by','bz']):
        ax.plot(t_b, ba_hist[:,i], label=lbl)
    if true_ba is not None:
        for i in range(3):
            ax.axhline(true_ba[i], color=f'C{i}', ls='--', alpha=.3)
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Acc Bias (m/s2)')
    ax.legend(); ax.grid(True); ax.set_title('Accel Bias Estimation')

    ax = axes[1,2]
    ax.plot(np.arange(len(cf_bias))*0.01, cf_bias, 'b-')
    ax.axhline(true_gb, color='r', ls='--', alpha=.5, label=f'true={true_gb:.3f}')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Gyro Bias z (rad/s)')
    ax.legend(); ax.grid(True); ax.set_title('CF Gyro Bias z')

    plt.tight_layout()
    return fig


# ============================================================
#  Main
# ============================================================

def main():
    np.random.seed(42)
    DT = 0.01
    DUR = 60
    TRUE_BA = np.array([0.15, -0.10, 0.08])
    TRUE_GB = 0.03

    figs = []

    # Test 1: Straight
    print("\n  Test 1: Straight line")
    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_straight(DUR, DT)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, 1.5)
    est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist = run_sim(
        "Straight", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Straight", tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, TRUE_GB, ba_hist, TRUE_BA))

    # Test 2: Rectangle
    print("\n  Test 2: Rectangle")
    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_rectangle(DUR, DT)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, 1.5)
    est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist = run_sim(
        "Rectangle", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Rectangle", tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, TRUE_GB, ba_hist, TRUE_BA))

    # Test 3: Circle
    print("\n  Test 3: Circle")
    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_circle(DUR, DT, radius=15)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, 1.5)
    est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist = run_sim(
        "Circle", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Circle", tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, TRUE_GB, ba_hist, TRUE_BA))

    # Test 4: GPS outage (with mag)
    print("\n  Test 4: GPS outage + mag (20-40s)")
    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_circle(DUR, DT, radius=15)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, 1.5)
    est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist = run_sim(
        "GPS Outage + Mag", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB,
        gps_loss_ranges=[(2000, 4000)])
    figs.append(plot_result("GPS Outage + Mag", tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, TRUE_GB, ba_hist, TRUE_BA))

    # Test 5: Noisy GPS (with mag)
    print("\n  Test 5: Noisy GPS + mag (sigma=5m)")
    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_circle(DUR, DT, radius=15)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, 5.0)
    est, pstd, pe, ge, ye, yaw_hist, cf_bias, ba_hist = run_sim(
        "Noisy GPS + Mag", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Noisy GPS + Mag", tx, ty, est, gx, gy, pe, ge, ye, pstd, yaw_hist, cf_bias, TRUE_GB, ba_hist, TRUE_BA))

    for i, fig in enumerate(figs):
        fname = f'eskf_result_{i+1}.png'
        fig.savefig(fname, dpi=120, bbox_inches='tight')
        print(f"  Saved: {fname}")

    print("\nDone.")


if __name__ == '__main__':
    main()

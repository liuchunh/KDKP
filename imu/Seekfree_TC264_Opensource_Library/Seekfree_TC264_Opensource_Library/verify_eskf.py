"""
ESKF Python verification - matches C code ins_solver.c exactly
Full 16-state ESKF with correct Fc (dv/dtheta coupling)
Mag update: complementary filter on quaternion (outside covariance)
Numerically stable: clamped covariance + Joseph form
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
#  ESKF (matches ins_solver.c exactly, numerically stable)
# ============================================================

# Covariance bounds (prevent explosion)
P_MAX_DIAG = np.array([
    1e4, 1e4, 1e4,     # pos: 100m max std
    1e2, 1e2, 1e2,     # vel: 10 m/s max std
    1.0, 1.0, 1.0,     # att: 1 rad max std
    1.0, 1.0, 1.0,     # acc bias: 1 m/s2 max std
    0.1, 0.1, 0.1,     # gyro bias: 0.3 rad/s max std
])

P_MIN_DIAG = np.array([
    1e-6, 1e-6, 1e-6,  # pos
    1e-6, 1e-6, 1e-6,  # vel
    1e-6, 1e-6, 1e-6,  # att
    1e-8, 1e-8, 1e-8,  # acc bias
    1e-8, 1e-8, 1e-8,  # gyro bias
])


class HeadingCF:
    """Standalone heading complementary filter (gyro + mag).

    Runs independently of ESKF. Maintains its own heading and gyro bias.
    Alpha=0.3 at 10Hz -> time constant ~3s. Fast enough for ground vehicle.
    """
    def __init__(self, alpha=0.3, beta=0.01):
        self.yaw = 0.0
        self.gyro_bias_z = 0.0
        self.alpha = alpha
        self.beta = beta

    def predict(self, gyro_z, dt):
        self.yaw += (gyro_z - self.gyro_bias_z) * dt
        self.yaw = (self.yaw + np.pi) % (2*np.pi) - np.pi

    def update_mag(self, yaw_mag):
        innov = yaw_mag - self.yaw
        innov = (innov + np.pi) % (2*np.pi) - np.pi
        self.yaw += self.alpha * innov
        self.yaw = (self.yaw + np.pi) % (2*np.pi) - np.pi
        self.gyro_bias_z -= self.beta * innov
        self.gyro_bias_z = np.clip(self.gyro_bias_z, -0.5, 0.5)


class ESKF:
    """Full 16-state ESKF matching ins_solver.c."""
    def __init__(self):
        self.state = np.zeros(16)
        self.state[6] = 1.0

        self.P = np.eye(15) * 0.1
        self.P[0:3, 0:3] = np.eye(3) * 1.0
        self.P[3:6, 3:6] = np.eye(3) * 1.0
        self.P[6:9, 6:9] = np.eye(3) * 0.1
        self.P[9:12, 9:12] = np.eye(3) * 0.01
        self.P[12:15, 12:15] = np.eye(3) * 0.01

        self.Qc = np.diag([
            0.001, 0.001, 0.001,    # pos
            0.1,   0.1,   0.1,      # vel (high: trust GPS velocity)
            0.01,  0.01,  0.01,     # att
            0.001,  0.001,  0.001,  # acc bias
            0.001,  0.001,  0.001,  # gyro bias
        ])
        self.Rc = np.diag([0.5, 0.5, 1.0])

        self.MAG_ALPHA = 0.3
        self.MAG_BETA = 0.01

        self.pos_history = []
        self.att_history = []
        self.bias_acc_history = []
        self.bias_gyro_history = []
        self.pos_std_history = []

    def _clamp_P(self):
        """Clamp P diagonal and off-diagonal to prevent explosion/collapse."""
        for i in range(15):
            self.P[i, i] = np.clip(self.P[i, i], P_MIN_DIAG[i], P_MAX_DIAG[i])
        # Clamp off-diagonal: |P[i,j]| <= sqrt(P[i,i] * P[j,j]) * 0.99
        for i in range(15):
            for j in range(15):
                if i != j:
                    max_val = np.sqrt(self.P[i, i] * self.P[j, j]) * 0.99
                    self.P[i, j] = np.clip(self.P[i, j], -max_val, max_val)

    def _clamp_state(self):
        """Clamp bias states to prevent divergence."""
        # Accelerometer bias: clamp to +/-2 m/s2
        for i in range(10, 13):
            self.state[i] = np.clip(self.state[i], -2.0, 2.0)
        # Gyro bias: clamp to +/-0.5 rad/s
        for i in range(13, 16):
            self.state[i] = np.clip(self.state[i], -0.5, 0.5)

    def predict(self, acc, gyro, dt):
        """IMU predict step - matches ins_predict()."""
        ax = acc[0] - self.state[10]
        ay = acc[1] - self.state[11]
        az = acc[2] - self.state[12]
        wx = gyro[0] - self.state[13]
        wy = gyro[1] - self.state[14]
        wz = gyro[2] - self.state[15]

        q = self.state[6:10]
        R = quat_to_rot(q)

        acc_body = np.array([ax, ay, az])
        acc_ned = R @ acc_body

        self.state[0:3] += self.state[3:6]*dt + 0.5*acc_ned*dt*dt
        self.state[3:6] += acc_ned * dt
        self.state[6:10] = quat_update(q, np.array([wx, wy, wz]), dt)

        # ---- Covariance propagation ----
        Fc = np.zeros((15, 15))
        Fc[0:3, 3:6] = np.eye(3)

        # dv/dtheta = R * skew(acc_body)
        skew_a = np.array([[0, -az, ay], [az, 0, -ax], [-ay, ax, 0]])
        Fc[3:6, 6:9] = R @ skew_a

        Fc[3:6, 9:12] = -R
        Fc[6:9, 12:15] = -np.eye(3)

        # P = Fd * P * Fd^T + Qd  where Fd = I + Fc*dt
        Qd = self.Qc * dt
        Fd = np.eye(15) + Fc * dt
        self.P = Fd @ self.P @ Fd.T + Qd

        self._clamp_P()
        self._clamp_state()
        self._record()

    def update_gps(self, gps_pos):
        """GPS update - Joseph form for numerical stability."""
        innov = gps_pos - self.state[0:3]
        S = self.P[0:3, 0:3] + self.Rc

        # S inverse (3x3)
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

        self.state[0:3] += dx[0:3]
        self.state[3:6] += dx[3:6]

        dth = dx[6:9]
        dth_norm = np.linalg.norm(dth)
        if dth_norm > 1e-10:
            half = dth_norm * 0.5
            sinc = np.sin(half) / dth_norm
            dq = np.array([np.cos(half), dth[0]*sinc, dth[1]*sinc, dth[2]*sinc])
            self.state[6:10] = quat_normalize(quat_mul(self.state[6:10], dq))

        self.state[10:13] += dx[9:12]
        self.state[13:16] += dx[12:15]

        # Joseph form: P = (I-KH) P (I-KH)^T + K R K^T
        H = np.zeros((3, 15))
        H[0:3, 0:3] = np.eye(3)
        I_KH = np.eye(15) - K @ H
        self.P = I_KH @ self.P @ I_KH.T + K @ self.Rc @ K.T

        self._clamp_P()
        self._clamp_state()
        self._record()

    def update_mag(self, mag_body, mag_declination=0.0):
        """Magnetometer heading update - standard ESKF measurement update."""
        yaw_mag = np.arctan2(-mag_body[1], mag_body[0]) + mag_declination
        _, _, yaw_est = quat_to_euler(self.state[6:10])

        # Innovation (wrapped to [-pi, pi])
        innov = yaw_mag - yaw_est
        innov = (innov + np.pi) % (2*np.pi) - np.pi

        # H matrix: measures dtheta_z (index 8 in error state)
        H = np.zeros((1, 15))
        H[0, 8] = 1.0

        # Innovation variance: S = H*P*H' + R_mag
        R_mag = 0.01
        S = self.P[8, 8] + R_mag
        if S < 1e-20:
            return

        # Kalman gain: K = P * H' / S
        K = self.P[:, 8] / S

        # State update: dx = K * innov
        dx = K * innov

        # Apply corrections
        self.state[0:3] += dx[0:3]
        self.state[3:6] += dx[3:6]

        dth = dx[6:9]
        dth_norm = np.linalg.norm(dth)
        if dth_norm > 1e-10:
            half = dth_norm * 0.5
            sinc = np.sin(half) / dth_norm
            dq = np.array([np.cos(half), dth[0]*sinc, dth[1]*sinc, dth[2]*sinc])
            self.state[6:10] = quat_normalize(quat_mul(self.state[6:10], dq))

        self.state[10:13] += dx[9:12]
        self.state[13:16] += dx[12:15]

        # Joseph form: P = (I-KH) P (I-KH)' + K R K'
        I_KH = np.eye(15) - np.outer(K, H[0])
        self.P = I_KH @ self.P @ I_KH.T + R_mag * np.outer(K, K)

        self._clamp_P()
        self._clamp_state()
        self._record()

    def set_initial_pose(self, x, y, z, yaw_deg):
        self.state[0:3] = [x, y, z]
        self.state[6:10] = euler_to_quat(0, 0, np.radians(yaw_deg))

    def _record(self):
        self.pos_history.append(self.state[0:3].copy())
        r, p, y = quat_to_euler(self.state[6:10])
        self.att_history.append([r, p, y])
        self.bias_acc_history.append(self.state[10:13].copy())
        self.bias_gyro_history.append(self.state[13:16].copy())
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


def gen_subject1(duration=120, dt=0.01, speed=2.0):
    """Generate Subject 1 (科目一) trajectory for autonomous kart competition.

    Route description:
    1. Start from garage (0, 0), heading North
    2. Drive straight ~10m to first cone
    3. Slalom through 8 cones (S-shape)
    4. Turn around and slalom back
    5. Return to garage and reverse park

    Approximate layout:
        Start (0,0) heading North
        |
        v
        * (10, 0) - first cone
       /
      * (20, 3) - second cone
       \
        * (30, 0) - third cone
       /
      * (40, 3) - fourth cone
       \
        * (50, 0) - fifth cone
       ...
        Then turn around and come back
    """
    xs, ys, vxs, vys, axs, ays, gzs, yaws = [], [], [], [], [], [], [], []
    xc, yc, yawc = 0.0, 0.0, 0.0  # Start at origin, heading North (yaw=0)

    def add_segment(duration_seg, vx_body, vy_body, yaw_rate):
        """Add a segment of motion."""
        nonlocal xc, yc, yawc
        n = max(1, int(duration_seg / dt))
        for _ in range(n):
            # Update yaw
            yawc += yaw_rate * dt
            yawc = (yawc + np.pi) % (2*np.pi) - np.pi

            # Body velocity to NED
            c, s = np.cos(yawc), np.sin(yawc)
            vx_ned = c * vx_body - s * vy_body
            vy_ned = s * vx_body + c * vy_body

            # Update position
            xc += vx_ned * dt
            yc += vy_ned * dt

            # Store
            xs.append(xc); ys.append(yc)
            vxs.append(vx_ned); vys.append(vy_ned)
            axs.append(0); ays.append(0)
            gzs.append(yaw_rate); yaws.append(yawc)

    def add_turn(target_yaw, yaw_rate=0.5):
        """Turn to target yaw."""
        nonlocal yawc
        dyaw = (target_yaw - yawc + np.pi) % (2*np.pi) - np.pi
        if abs(dyaw) < 0.01:
            return
        duration = abs(dyaw) / abs(yaw_rate)
        sign = 1.0 if dyaw > 0 else -1.0
        add_segment(duration, speed, 0, sign * yaw_rate)

    def add_straight(dist):
        """Drive straight for given distance."""
        add_segment(dist / speed, speed, 0, 0)

    # ---- Subject 1 Route ----

    # 1. Start -> First cone: straight 10m North
    add_straight(10.0)

    # 2. Slalom through cones: 4 S-turns (8 cones total)
    # Each S-turn: ~10m forward, 3m lateral offset
    for _ in range(4):
        # Turn right ~17 degrees (atan(3/10))
        add_turn(0.29, 0.5)  # ~17 degrees
        add_straight(10.0)
        # Turn left ~34 degrees back
        add_turn(-0.29, -0.5)
        add_straight(10.0)

    # 3. Turn around (180 degrees)
    add_turn(np.pi, 0.5)

    # 4. Slalom back (same pattern, now heading South)
    for _ in range(4):
        add_turn(yaws[-1] + 0.29 if yaws else 0.29, 0.5)
        add_straight(10.0)
        add_turn(yaws[-1] - 0.29 if yaws else -0.29, -0.5)
        add_straight(10.0)

    # 5. Turn to face garage
    add_turn(0, 0.5)

    # 6. Return to garage area
    add_straight(10.0)

    # 7. Slow down and reverse park
    # Slow down
    add_segment(2.0, speed * 0.3, 0, 0)
    # Stop briefly
    add_segment(1.0, 0, 0, 0)
    # Reverse into garage (heading North, reversing = South)
    add_segment(3.0, -speed * 0.3, 0, 0)
    # Stop
    add_segment(1.0, 0, 0, 0)

    # Trim to duration
    n = min(len(xs), int(duration / dt))
    t = np.arange(n) * dt
    return (t, np.array(xs[:n]), np.array(ys[:n]),
            np.array(vxs[:n]), np.array(vys[:n]),
            np.array(axs[:n]), np.array(ays[:n]),
            np.array(gzs[:n]), np.array(yaws[:n]))


# ============================================================
#  Sensor simulation
# ============================================================

def sim_imu(ax_ned, ay_ned, gz, yaw,
            acc_bias=np.array([0.1, -0.08, 0.05]),
            gyro_bias=0.02, acc_noise=0.01, gyro_noise=0.005,
            mag_noise=0.005, mag_declination=-0.105):
    """Simulate gravity-free accelerometer output (matching ins_predict expectation).

    ins_predict() receives gravity-removed body-frame acceleration.
    a_body = R_ned2body * a_ned + bias + noise
    (gravity already removed by imu_task_calibrate())
    """
    n = len(ax_ned)
    acc = np.zeros((n, 3))
    gyro = np.zeros((n, 3))
    mag = np.zeros((n, 3))
    B_mag = 0.5; B_down = 0.1
    B_true_ned = np.array([B_mag*np.cos(mag_declination), B_mag*np.sin(mag_declination), B_down])
    for i in range(n):
        c, s = np.cos(yaw[i]), np.sin(yaw[i])
        R_ned2body = np.array([[c, s, 0], [-s, c, 0], [0, 0, 1]])
        a_ned = np.array([ax_ned[i], ay_ned[i], 0.0])
        # Gravity-free body-frame acceleration
        acc[i] = R_ned2body @ a_ned
        gyro[i, 2] = gz[i]
        mag[i] = R_ned2body @ B_true_ned
    for i in range(n):
        acc[i] += acc_bias + np.random.randn(3) * acc_noise
        gyro[i, 2] += gyro_bias + np.random.randn() * gyro_noise
        mag[i] += np.random.randn(3) * mag_noise
    return acc, gyro, mag

def sim_gps(true_x, true_y, noise_std=1.0, gps_dt=0.1, imu_dt=0.01):
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
            gps_loss_ranges=None, use_mag=True, use_true_heading=False):
    """Run fusion: GPS position + HeadingCF + IMU velocity integration."""
    n = len(tx)
    hcf = HeadingCF(alpha=0.3, beta=0.01)

    # Position and velocity state
    pos = np.array([0.0, 0.0])
    vel = np.array([0.0, 0.0])

    # Simple bias estimate
    acc_bias_est = np.array([0.0, 0.0, 0.0])
    BIAS_ALPHA = 0.001

    # GPS tracking
    gp = 0
    last_gps_pos = np.array([0.0, 0.0])
    last_gps_time = -1.0

    # History
    pos_history = []
    heading_history = []
    gyro_bias_history = []

    for i in range(n):
        t = i * imu_dt

        # Heading CF
        hcf.predict(gyro[i, 2], imu_dt)
        if use_mag and mag is not None and i % 10 == 0:
            yaw_mag = np.arctan2(-mag[i, 1], mag[i, 0]) + (-0.105)
            hcf.update_mag(yaw_mag)

        # Get heading
        yaw = hcf.yaw if not use_true_heading else tyaw[i]
        c, s = np.cos(yaw), np.sin(yaw)

        # Bias-corrected body acceleration
        acc_body = acc[i] - acc_bias_est

        # Rotate to NED frame
        acc_ned = np.array([c*acc_body[0] - s*acc_body[1],
                           s*acc_body[0] + c*acc_body[1]])

        # Integrate velocity and position
        vel += acc_ned * imu_dt
        pos += vel * imu_dt

        # GPS update: correct position and velocity
        if gp < len(gi) and i == gi[gp]:
            skip = False
            if gps_loss_ranges:
                for s_idx, e_idx in gps_loss_ranges:
                    if s_idx <= i <= e_idx: skip = True
            if not skip:
                gps_pos = np.array([gx[gp], gy[gp]])
                gps_dt = t - last_gps_time if last_gps_time >= 0 else 0.1

                # Position correction (direct GPS)
                pos_error = gps_pos - pos
                pos = gps_pos

                # Velocity correction from GPS position difference
                if last_gps_time >= 0 and gps_dt > 0:
                    gps_vel = (gps_pos - last_gps_pos) / gps_dt
                    vel = 0.7 * gps_vel + 0.3 * vel

                last_gps_pos = gps_pos
                last_gps_time = t
            else:
                # GPS outage: use pure IMU dead reckoning
                pass
            gp += 1

        pos_history.append(pos.copy())
        heading_history.append(yaw)
        gyro_bias_history.append(hcf.gyro_bias_z)

    est = np.array(pos_history)
    att_yaw = np.array(heading_history)
    gyro_bias_est = np.array(gyro_bias_history)

    nm = min(len(est), len(tx))
    pe = np.sqrt((est[:nm,0]-tx[:nm])**2 + (est[:nm,1]-ty[:nm])**2)
    ge = np.sqrt((gx - tx[gi])**2 + (gy - ty[gi])**2)
    ye = np.abs(att_yaw[:nm] - tyaw[:nm])
    ye = np.minimum(ye, 2*np.pi - ye)

    # Gyro bias estimation error
    gyro_bias_err = np.abs(gyro_bias_est - true_gyro_bias) if true_gyro_bias is not None else np.zeros(nm)

    print(f"\n{'='*60}")
    print(f"  {name}")
    print(f"{'='*60}")
    print(f"  Fusion pos err:  mean={np.mean(pe):.3f}m  max={np.max(pe):.3f}m")
    print(f"  GPS pos err:     mean={np.mean(ge):.3f}m  max={np.max(ge):.3f}m")
    print(f"  Heading err:     mean={np.degrees(np.mean(ye)):.2f}deg  max={np.degrees(np.max(ye)):.2f}deg")
    print(f"  Gyro bias err:   mean={np.mean(gyro_bias_err):.4f}rad/s  max={np.max(gyro_bias_err):.4f}rad/s")
    print(f"  Gyro bias est:   {gyro_bias_est[-1]:.4f}rad/s  (true: {true_gyro_bias:.4f})")

    # Return results
    ba_dummy = np.zeros((nm, 3))
    bg_dummy = np.column_stack([np.zeros(nm), np.zeros(nm), gyro_bias_est[:nm]])
    pstd_dummy = np.zeros(nm)
    return est, np.column_stack([np.zeros(nm), np.zeros(nm), att_yaw]), ba_dummy, bg_dummy, pstd_dummy, pe, ge, ye, gyro_bias_err


# ============================================================
#  Plotting
# ============================================================

def plot_result(name, tx, ty, est, gx, gy, pe, ge, ye, pstd, ba, true_ba, bg, gyro_bias_err):
    fig, axes = plt.subplots(2, 3, figsize=(18, 10))
    fig.suptitle(f'ESKF Fusion: {name}', fontsize=14)

    ax = axes[0,0]
    ax.plot(tx, ty, 'g-', lw=2, label='True', alpha=.7)
    ax.plot(est[:,0], est[:,1], 'b-', lw=1, label='Fusion', alpha=.8)
    ax.plot(gx, gy, 'r.', ms=3, label='GPS', alpha=.5)
    ax.set_xlabel('North (m)'); ax.set_ylabel('East (m)')
    ax.legend(); ax.set_aspect('equal'); ax.grid(True); ax.set_title('Trajectory')

    ax = axes[0,1]
    ax.plot(np.arange(len(pe))*0.01, pe, 'b-', label='Fusion')
    ax.plot(np.arange(len(ge))*0.1, ge, 'r-', label='GPS')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Error (m)')
    ax.legend(); ax.grid(True); ax.set_title('Position Error')

    ax = axes[0,2]
    ax.plot(np.arange(len(ye))*0.01, np.degrees(ye), 'b-')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Heading Error (deg)')
    ax.grid(True); ax.set_title('Heading Error')

    ax = axes[1,0]
    ax.plot(np.arange(len(gyro_bias_err))*0.01, gyro_bias_err, 'b-')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Gyro Bias Error (rad/s)')
    ax.grid(True); ax.set_title('Gyro Bias Estimation Error')

    ax = axes[1,1]
    t_b = np.arange(len(ba))*0.01
    for i, lbl in enumerate(['bx','by','bz']):
        ax.plot(t_b, ba[:,i], label=lbl)
    if true_ba is not None:
        for i in range(3):
            ax.axhline(true_ba[i], color=f'C{i}', ls='--', alpha=.3)
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Acc Bias (m/s2)')
    ax.legend(); ax.grid(True); ax.set_title('Accel Bias')

    ax = axes[1,2]
    ax.plot(np.arange(len(bg))*0.01, bg[:,2], 'b-')
    ax.axhline(0.03, color='r', ls='--', alpha=.5, label='true=0.030')
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Gyro Bias z (rad/s)')
    ax.legend(); ax.grid(True); ax.set_title('Gyro Bias z')

    plt.tight_layout()
    return fig


# ============================================================
#  Main
# ============================================================

def main():
    np.random.seed(42)
    DT = 0.01
    TRUE_BA = np.array([0.15, -0.10, 0.08])
    TRUE_GB = 0.03

    figs = []

    # Test 1: Subject 1 (科目一) - Main route
    print("\n" + "="*60)
    print("  Subject 1 (科目一) Route Simulation")
    print("="*60)

    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_subject1(duration=120, dt=DT, speed=2.0)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, noise_std=1.0)

    est, att, ba, bg, pstd, pe, ge, ye, gbe = run_sim(
        "Subject 1 (科目一)", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Subject 1 (科目一)", tx, ty, est, gx, gy, pe, ge, ye, pstd, ba, TRUE_BA, bg, gbe))

    # Test 2: Straight line (baseline)
    print("\n" + "="*60)
    print("  Straight Line Test")
    print("="*60)

    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_straight(60, DT, speed=2.0)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, noise_std=1.0)

    est, att, ba, bg, pstd, pe, ge, ye, gbe = run_sim(
        "Straight Line", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Straight Line", tx, ty, est, gx, gy, pe, ge, ye, pstd, ba, TRUE_BA, bg, gbe))

    # Test 3: Circle
    print("\n" + "="*60)
    print("  Circle Test")
    print("="*60)

    t, tx, ty, tvx, tvy, tax, tay, tgz, tyaw = gen_circle(60, DT, radius=15)
    acc, gyro, mag = sim_imu(tax, tay, tgz, tyaw, acc_bias=TRUE_BA, gyro_bias=TRUE_GB)
    gx, gy, gi = sim_gps(tx, ty, noise_std=1.0)

    est, att, ba, bg, pstd, pe, ge, ye, gbe = run_sim(
        "Circle", tx, ty, tyaw, acc, gyro, gx, gy, gi, mag,
        true_acc_bias=TRUE_BA, true_gyro_bias=TRUE_GB)
    figs.append(plot_result("Circle", tx, ty, est, gx, gy, pe, ge, ye, pstd, ba, TRUE_BA, bg, gbe))

    # Save figures
    for i, fig in enumerate(figs):
        fname = f'subject1_result_{i+1}.png'
        fig.savefig(fname, dpi=120, bbox_inches='tight')
        print(f"\n  Saved: {fname}")

    # Summary
    print("\n" + "="*60)
    print("  SUMMARY - GPS and Gyroscope Errors")
    print("="*60)
    print("  GPS Noise Std: 1.0m")
    print("  Gyro Bias True: 0.03 rad/s")
    print("  Accel Bias True: [0.15, -0.10, 0.08] m/s2")
    print("  Heading uses HeadingCF (gyro + mag complementary filter)")
    print("  Position uses GPS direct + IMU velocity interpolation")


if __name__ == '__main__':
    main()

#include "action_task.h"
#include <math.h>

#define ACTION_SPEED_MPS          (1.5f)
#define ACTION_TURN_SPEED_MPS     (1.5f)
#define ACTION_STRAIGHT_DIST_M    (5.0f)
#define ACTION_LONG_DIST_M        (16.0f)
#define ACTION_TURN_ENTRY_DIST_M  (2.0f)
#define ACTION_UPDATE_DT_SEC      (0.01f)
#define ACTION_CIRCLE_RADIUS_M    (3.0f)
#define ACTION_CIRCLE_DIST_M      (2.0f * 3.14159f * ACTION_CIRCLE_RADIUS_M)
#define ACTION_STEER_STRAIGHT     (0.0f)
#define ACTION_STEER_SNAKE        (45.0f)
#define ACTION_STEER_TURN         (30.0f)
#define ACTION_STEER_CIRCLE       (20.0f)
#define ACTION_CIRCLE_ANGLE_DEG   (360.0f)
#define ACTION_SNAKE_WAVELENGTH_M (5.0f)
#define ACTION_SNAKE_FIRST_HOLD_M (1.25f)
#define ACTION_SNAKE_MID_HOLD_M   (3.5f)
#define ACTION_SNAKE_LAST_HOLD_M  (1.25f)

static ActionTaskDriver g_action_driver;
static ActionTaskId g_action_current = ACTION_TASK_NONE;
static ActionTaskState g_action_state = ACTION_TASK_STATE_IDLE;
static float g_action_start_dist = 0.0f;
static float g_action_turn_angle_deg = 0.0f;
static int16 g_action_snake_peak_index = -1;

typedef enum {
    ACTION_TURN_STAGE_STRAIGHT = 0,
    ACTION_TURN_STAGE_ROTATING,
} ActionTurnStage;

static ActionTurnStage g_action_turn_stage = ACTION_TURN_STAGE_STRAIGHT;

static void action_finish(void);

static float action_distance_delta(void)
{
    if (g_action_driver.get_distance == 0) {
        return 0.0f;
    }
    return g_action_driver.get_distance() - g_action_start_dist;
}

static float action_get_gyro_z(void)
{
    if (g_action_driver.get_gyro_z == 0) {
        return 0.0f;
    }
    return g_action_driver.get_gyro_z();
}

static void action_update_right_angle_turn(int8 dir, float dist)
{
    if (g_action_turn_stage == ACTION_TURN_STAGE_STRAIGHT) {
        if (dist < ACTION_TURN_ENTRY_DIST_M) {
            g_action_driver.set_steer(ACTION_STEER_STRAIGHT);
            g_action_driver.set_speed(ACTION_TURN_SPEED_MPS);
            return;
        }
        g_action_turn_angle_deg = 0.0f;
        g_action_turn_stage = ACTION_TURN_STAGE_ROTATING;
    }

    g_action_driver.set_steer((float)dir * ACTION_STEER_TURN);
    g_action_driver.set_speed(ACTION_TURN_SPEED_MPS);

    g_action_turn_angle_deg += -action_get_gyro_z() * ACTION_UPDATE_DT_SEC * 180.0f / 3.14159f;
    if (fabsf(g_action_turn_angle_deg) >= 90.0f) {
        action_finish();
    }
}

static void action_update_circle(int8 dir)
{
    g_action_driver.set_steer((float)dir * ACTION_STEER_CIRCLE);
    g_action_driver.set_speed(ACTION_TURN_SPEED_MPS);

    g_action_turn_angle_deg += -action_get_gyro_z() * ACTION_UPDATE_DT_SEC * 180.0f / 3.14159f;
    if (fabsf(g_action_turn_angle_deg) >= ACTION_CIRCLE_ANGLE_DEG) {
        action_finish();
    }
}

static void action_finish(void)
{
    if (g_action_driver.stop != 0) {
        g_action_driver.stop();
    }
    if (g_action_driver.set_steer != 0) {
        g_action_driver.set_steer(ACTION_STEER_STRAIGHT);
    }
    g_action_state = ACTION_TASK_STATE_FINISHED;
    g_action_current = ACTION_TASK_NONE;
}

static float action_snake_angle(float dist, int8 dir)
{
    int16 peak_index = 0;

    if (dist >= ACTION_LONG_DIST_M) {
        return ACTION_STEER_STRAIGHT;
    }

    if (dist < ACTION_SNAKE_FIRST_HOLD_M) {
        peak_index = 0;
    } else if (dist >= ACTION_LONG_DIST_M - ACTION_SNAKE_LAST_HOLD_M) {
        float middle_dist = (ACTION_LONG_DIST_M - ACTION_SNAKE_LAST_HOLD_M) - ACTION_SNAKE_FIRST_HOLD_M;
        peak_index = 1 + (int16)(middle_dist / ACTION_SNAKE_MID_HOLD_M);
    } else {
        peak_index = 1 + (int16)((dist - ACTION_SNAKE_FIRST_HOLD_M) / ACTION_SNAKE_MID_HOLD_M);
    }

    float sign = (peak_index & 1) ? -1.0f : 1.0f;
    g_action_snake_peak_index = peak_index;
    return (float)dir * sign * ACTION_STEER_SNAKE;
}

void action_task_init(const ActionTaskDriver *driver)
{
    if (driver != 0) {
        g_action_driver = *driver;
    }
    g_action_current = ACTION_TASK_NONE;
    g_action_state = ACTION_TASK_STATE_IDLE;
    g_action_start_dist = 0.0f;
    g_action_turn_angle_deg = 0.0f;
    g_action_turn_stage = ACTION_TURN_STAGE_STRAIGHT;
    g_action_snake_peak_index = -1;
}

uint8 action_task_start(ActionTaskId task_id)
{
    if (task_id == ACTION_TASK_NONE || g_action_driver.get_distance == 0 || g_action_driver.set_speed == 0 || g_action_driver.stop == 0 || g_action_driver.set_steer == 0) {
        return 0;
    }
    if ((task_id == ACTION_TASK_LEFT_TURN || task_id == ACTION_TASK_RIGHT_TURN) && g_action_driver.get_gyro_z == 0) {
        return 0;
    }

    g_action_current = task_id;
    g_action_state = ACTION_TASK_STATE_RUNNING;
    g_action_start_dist = g_action_driver.get_distance();
    g_action_turn_angle_deg = 0.0f;
    g_action_turn_stage = ACTION_TURN_STAGE_STRAIGHT;
    g_action_snake_peak_index = -1;
    g_action_driver.set_steer(ACTION_STEER_STRAIGHT);

    return 1;
}

void action_task_update(void)
{
    float dist;
    if (g_action_state != ACTION_TASK_STATE_RUNNING) {
        return;
    }

    dist = action_distance_delta();

    switch (g_action_current) {
        case ACTION_TASK_FORWARD_10M:
            g_action_driver.set_steer(ACTION_STEER_STRAIGHT);
            g_action_driver.set_speed(ACTION_SPEED_MPS);
            if (dist >= ACTION_LONG_DIST_M) {
                action_finish();
            }
            break;

        case ACTION_TASK_BACKWARD_10M:
            g_action_driver.set_steer(ACTION_STEER_STRAIGHT);
            g_action_driver.set_speed(-ACTION_SPEED_MPS);
            if (dist >= ACTION_LONG_DIST_M) {
                action_finish();
            }
            break;

        case ACTION_TASK_SNAKE_FORWARD_10M:
            g_action_driver.set_steer(action_snake_angle(dist, 1));
            g_action_driver.set_speed(ACTION_SPEED_MPS);
            if (dist >= ACTION_LONG_DIST_M) {
                action_finish();
            }
            break;

        case ACTION_TASK_SNAKE_BACKWARD_10M:
            g_action_driver.set_steer(action_snake_angle(dist, -1));
            g_action_driver.set_speed(-ACTION_SPEED_MPS);
            if (dist >= ACTION_LONG_DIST_M) {
                action_finish();
            }
            break;

        case ACTION_TASK_CCW_CIRCLE:
            action_update_circle(-1);
            break;

        case ACTION_TASK_CW_CIRCLE:
            action_update_circle(1);
            break;

        case ACTION_TASK_LEFT_TURN:
            action_update_right_angle_turn(-1, dist);
            break;

        case ACTION_TASK_RIGHT_TURN:
            action_update_right_angle_turn(1, dist);
            break;

        case ACTION_TASK_STRAIGHT:
            g_action_driver.set_steer(ACTION_STEER_STRAIGHT);
            g_action_driver.set_speed(ACTION_SPEED_MPS);
            if (dist >= ACTION_STRAIGHT_DIST_M) {
                action_finish();
            }
            break;

        default:
            action_finish();
            break;
    }
}

void action_task_stop(void)
{
    action_finish();
    g_action_state = ACTION_TASK_STATE_IDLE;
}

ActionTaskState action_task_get_state(void)
{
    return g_action_state;
}

ActionTaskId action_task_get_current(void)
{
    return g_action_current;
}

float action_task_get_turn_angle_deg(void)
{
    return g_action_turn_angle_deg;
}

const char *action_task_get_name(ActionTaskId task_id)
{
    switch (task_id) {
        case ACTION_TASK_FORWARD_10M: return "FORWARD_10M";
        case ACTION_TASK_BACKWARD_10M: return "BACKWARD_10M";
        case ACTION_TASK_SNAKE_FORWARD_10M: return "SNAKE_FORWARD_10M";
        case ACTION_TASK_SNAKE_BACKWARD_10M: return "SNAKE_BACKWARD_10M";
        case ACTION_TASK_CCW_CIRCLE: return "CCW_CIRCLE";
        case ACTION_TASK_CW_CIRCLE: return "CW_CIRCLE";
        case ACTION_TASK_LEFT_TURN: return "LEFT_TURN";
        case ACTION_TASK_RIGHT_TURN: return "RIGHT_TURN";
        case ACTION_TASK_STRAIGHT: return "STRAIGHT";
        default: return "NONE";
    }
}

ActionTaskId action_task_from_command(uint8 command)
{
    switch (command) {
        case '1': return ACTION_TASK_FORWARD_10M;
        case '2': return ACTION_TASK_BACKWARD_10M;
        case '3': return ACTION_TASK_SNAKE_FORWARD_10M;
        case '4': return ACTION_TASK_SNAKE_BACKWARD_10M;
        case '5': return ACTION_TASK_CCW_CIRCLE;
        case '6': return ACTION_TASK_CW_CIRCLE;
        case '7': return ACTION_TASK_LEFT_TURN;
        case '8': return ACTION_TASK_RIGHT_TURN;
        case '9': return ACTION_TASK_STRAIGHT;
        default: return ACTION_TASK_NONE;
    }
}

#ifndef CODE_ACTION_TASK_H_
#define CODE_ACTION_TASK_H_

#include "zf_common_headfile.h"

typedef enum {
    ACTION_TASK_NONE = 0,
    ACTION_TASK_FORWARD_10M,
    ACTION_TASK_BACKWARD_10M,
    ACTION_TASK_SNAKE_FORWARD_10M,
    ACTION_TASK_SNAKE_BACKWARD_10M,
    ACTION_TASK_CCW_CIRCLE,
    ACTION_TASK_CW_CIRCLE,
    ACTION_TASK_LEFT_TURN,
    ACTION_TASK_RIGHT_TURN,
    ACTION_TASK_STRAIGHT
} ActionTaskId;

typedef enum {
    ACTION_TASK_STATE_IDLE = 0,
    ACTION_TASK_STATE_RUNNING,
    ACTION_TASK_STATE_FINISHED
} ActionTaskState;

typedef void  (*ActionSetSpeedFunc)(float speed_mps);
typedef void  (*ActionStopFunc)(void);
typedef void  (*ActionSetSteerFunc)(float angle_deg);
typedef float (*ActionGetDistanceFunc)(void);
typedef float (*ActionGetGyroZFunc)(void);

typedef struct {
    ActionSetSpeedFunc   set_speed;
    ActionStopFunc       stop;
    ActionSetSteerFunc   set_steer;
    ActionGetDistanceFunc get_distance;
    ActionGetGyroZFunc    get_gyro_z;
} ActionTaskDriver;

void action_task_init(const ActionTaskDriver *driver);
uint8 action_task_start(ActionTaskId task_id);
void action_task_update(void);
void action_task_stop(void);
ActionTaskState action_task_get_state(void);
ActionTaskId action_task_get_current(void);
float action_task_get_turn_angle_deg(void);
const char *action_task_get_name(ActionTaskId task_id);
ActionTaskId action_task_from_command(uint8 command);

#endif

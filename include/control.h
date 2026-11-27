#ifndef CONTROL_H
#define CONTROL_H

#include <stdio.h>
#include "ramp.h"
#include "chassis.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAD_LX_SCALE 4
#define PAD_LY_SCALE -4   
#define PAD_RX_SCALE -1   
#define PAD_RY_SCALE 1

typedef struct {
    ramp_t vx_ramp,vy_ramp;         // 斜坡函数
    chassis_spd_t spd_input;
    chassis_spd_t spd_output;

    uint8_t rotate_able;
    float err_yaw;
    float forward_yaw;  // 小陀螺行进的yaw角
    float rotate_speed;
} control_t;

extern control_t control;

void control_init();

#ifdef __cplusplus
}
#endif

#endif

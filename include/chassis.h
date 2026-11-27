#ifndef CHASSIS_H_
#define CHASSIS_H_

#include "encoder_motor.h"
#include "pid.h"
#include "ramp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 用户配置 */
#define CHASSIS_WHEEL_RADIUS    0.026f    // 直径5.2cm
#define CHASSIS_W               0.17f    // 轮距
#define CHASSIS_H               0.21f    // 轴距    
#define CHASSIS_MOTOR_RATIO     19      // 电机减速比
#define CHASSIS_MOTOR_PPR       11      // 电机编码器线数
#define CHASSIS_TASK_MS         10      // 底盘任务时间
#define CHASSIS_WHEEL_SPD_MAX   530.0f  // 单个轮子最大目标转速 (rpm)

/* 电机配置（定义在 chassis_task.c 中，避免头文件重复定义） */
extern const uint8_t chassis_motor_config[4][6];

/* 电机枚举 */
typedef enum {
    CHASSIS_MOTOR_FR,
    CHASSIS_MOTOR_FL,
    CHASSIS_MOTOR_BL,
    CHASSIS_MOTOR_BR,
    CHASSIS_MOTOR_NUM,
} chassis_motor_e;

typedef struct 
{
    encoder_motor_t motor;

    pid_ctrl_t spd_pid;
    float spd_get;
    float spd_set;
    int32_t compare;
} chassis_motor_t;

typedef struct {
    float vx,vy,vw; // m/s m/s rad/s
}chassis_spd_t;

typedef struct chassis_structure chassis_t;

struct chassis_structure
{
    chassis_motor_t motors[CHASSIS_MOTOR_NUM];
    
    pid_ctrl_t vx_pid,vy_pid,vw_pid;     // 
    chassis_spd_t spd_set;
    chassis_spd_t spd_get;
    chassis_spd_t spd_out;

    /* 底盘解算 */
    void (*chassis_pid_calc)(void);
    void (*chassis_spd_get)(void);
};

extern chassis_t chassis;

void chassis_init();

void chassis_task(void * args);

#ifdef __cplusplus
}
#endif

#endif /* CHASSIS_H_ */

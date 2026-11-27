#include <math.h>
#include <Arduino.h>
#include "chassis.h"
#include "imu.h"

chassis_t chassis;

// 线速度 v = rpm/60*2*pi*CHASSIS_WHEEL_RADIUS
const float RPM_TO_V = 2.0f*M_PI*CHASSIS_WHEEL_RADIUS/60.0f;

// chassis.motors[i].motor.enc_a_io = chassis_motor_config[i][0];
// chassis.motors[i].motor.enc_b_io = chassis_motor_config[i][1];
// chassis.motors[i].motor.pwm_1_io = chassis_motor_config[i][2];
// chassis.motors[i].motor.pwm_1_channel = chassis_motor_config[i][3];
// chassis.motors[i].motor.pwm_2_io = chassis_motor_config[i][4];
// chassis.motors[i].motor.pwm_2_channel = chassis_motor_config[i][5];

/* 电机配置 */
const uint8_t chassis_motor_config[4][6] = {
    //{47,21,  0,0,   35,1},
    //{48,45,  36,2,  37,3},
    {21,47,  35,0,   0,1},
    {45,48,  37,2,  36,3},
    {44,42,  40,6,  41,7},
    {43,2,   39,4,  38,5},
};

static void chassis_motor_pid_calc(chassis_motor_t * motor) {
    motor->spd_get = motor->motor.rpm;
    motor->compare = (int32_t)pid_calc(&motor->spd_pid,motor->spd_get,motor->spd_set);
    encoder_motor_set_compare(&motor->motor,(int16_t)motor->compare);
}

static void mecanum_get_speed() {
    float w[4]; // wheel_speed
    w[0] = chassis.motors[CHASSIS_MOTOR_FR].spd_get;
    w[1] = chassis.motors[CHASSIS_MOTOR_FL].spd_get;
    w[2] = chassis.motors[CHASSIS_MOTOR_BL].spd_get;
    w[3] = chassis.motors[CHASSIS_MOTOR_BR].spd_get;

    chassis.spd_get.vx = (w[0] + w[1] + w[2] + w[3]) / 4.0f;
    chassis.spd_get.vy = (w[0] - w[1] + w[2] - w[3]) / 4.0f;
    chassis.spd_get.vw = (w[0] - w[1] - w[2] + w[3]) / 4.0f;                // 线速度
    // chassis.spd_get.vw /= sqrt(CHASSIS_W*CHASSIS_W+CHASSIS_H*CHASSIS_H);    // 角速度 rad/s    
    // chassis.spd_get.vw = imu.wz;     //  陀螺仪的值更准
    // chassis.spd_get.vw = -imu.wz * (CHASSIS_W + CHASSIS_H) / (12.0f * CHASSIS_WHEEL_RADIUS);
}

static void mecanum_calc() {
    float w[CHASSIS_MOTOR_NUM];

    w[CHASSIS_MOTOR_FR] = chassis.spd_out.vx + chassis.spd_out.vy + chassis.spd_out.vw;
    w[CHASSIS_MOTOR_FL] = chassis.spd_out.vx - chassis.spd_out.vy - chassis.spd_out.vw;
    w[CHASSIS_MOTOR_BL] = chassis.spd_out.vx + chassis.spd_out.vy - chassis.spd_out.vw;
    w[CHASSIS_MOTOR_BR] = chassis.spd_out.vx - chassis.spd_out.vy + chassis.spd_out.vw;

    /* 四轮统一归一化：超过单轮最大转速时同比例缩小，保持合成速度方向不变 */
    float max_abs = 0.0f;
    for (int i = 0; i < CHASSIS_MOTOR_NUM; i++) {
        float a = fabsf(w[i]);
        if (a > max_abs) {
            max_abs = a;
        }
    }

    if (max_abs > CHASSIS_WHEEL_SPD_MAX) {
        float scale = CHASSIS_WHEEL_SPD_MAX / max_abs;
        for (int i = 0; i < CHASSIS_MOTOR_NUM; i++) {
            w[i] *= scale;
        }
    }

    for (int i = 0; i < CHASSIS_MOTOR_NUM; i++) {
        chassis.motors[i].spd_set = w[i];
    }
}

static void mecanum_pid_calc() {
    /* 运动pid解算 */
    chassis.spd_out.vx = pid_calc(&chassis.vx_pid,chassis.spd_get.vx,chassis.spd_set.vx);
    chassis.spd_out.vy = pid_calc(&chassis.vy_pid,chassis.spd_get.vy,chassis.spd_set.vy);
    chassis.spd_out.vw = pid_calc(&chassis.vw_pid,chassis.spd_get.vw,chassis.spd_set.vw);

    /* 麦轮解算 */
    mecanum_calc();    

    /* 电机pid解算 */
    for(int i=0;i<CHASSIS_MOTOR_NUM;i++) {
        if(fabs(chassis.motors[i].spd_set)<3) {
            encoder_motor_set_compare(&chassis.motors[i].motor,0);
        }
        else 
        chassis_motor_pid_calc(&chassis.motors[i]);
    }
}

void chassis_task(void * args) {
    TickType_t wake_time = xTaskGetTickCount();
    const TickType_t delay_time = pdMS_TO_TICKS(CHASSIS_TASK_MS);
    for(;;) {
        /* 获取电机速度 */
        for(int i=0;i<CHASSIS_MOTOR_NUM;i++) {
            encoder_motor_update(&chassis.motors[i].motor);
        } 
        
        /* 获取底盘速度 */
        if(chassis.chassis_spd_get)chassis.chassis_spd_get();
        
        /* 底盘速度解算 */
        if(chassis.chassis_pid_calc)chassis.chassis_pid_calc();

        vTaskDelayUntil(&wake_time, delay_time);
    }
}

void chassis_init() {
    /* 使用麦轮解算 */
    chassis.chassis_pid_calc = mecanum_pid_calc;
    chassis.chassis_spd_get  = mecanum_get_speed;

    /* 底盘电机初始化 */
    for(int i=0;i<CHASSIS_MOTOR_NUM;i++) {
        chassis.motors[i].motor.mode = ENCODER_MOTOR_2_PWM_MODE;
        chassis.motors[i].motor.ratio = CHASSIS_MOTOR_RATIO;
        chassis.motors[i].motor.ppr = CHASSIS_MOTOR_PPR;

        chassis.motors[i].motor.enc_a_io = chassis_motor_config[i][0];
        chassis.motors[i].motor.enc_b_io = chassis_motor_config[i][1];
        chassis.motors[i].motor.pwm_1_io = chassis_motor_config[i][2];
        chassis.motors[i].motor.pwm_1_channel = chassis_motor_config[i][3];
        chassis.motors[i].motor.pwm_2_io = chassis_motor_config[i][4];
        chassis.motors[i].motor.pwm_2_channel = chassis_motor_config[i][5];
        encoder_motor_init(&chassis.motors[i].motor);
        encoder_motor_set_compare(&chassis.motors[i].motor,100);
    }

    /* 初始化底盘pid */
    //ramp_init(&chassis->vx_ramp,,);
    //ramp_init(&chassis->vy_ramp,,);

    pid_struct_init(&chassis.vx_pid,530,100,
        0.4,12,0,0);
    pid_struct_init(&chassis.vy_pid,530,100,
        0.4,12,0,0);
    pid_struct_init(&chassis.vw_pid,530,100,
        0.4,12,0,0);    

    /* 初始化电机pid */
    for(int i=0;i<CHASSIS_MOTOR_NUM;i++) {
        pid_struct_init(&chassis.motors[i].spd_pid,
            1024,10,8,50,0,0);
    }

    xTaskCreate(chassis_task, "chassis_task", 1024 * 4,(void* ) 0, 10, NULL);
}

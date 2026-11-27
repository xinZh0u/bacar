#ifndef ENCODER_MOTOR_H
#define ENCODER_MOTOR_H

#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 驱动模式 */
typedef enum {
    ENCODER_MOTOR_1_PWM_MODE,     // 2IO控方向1PWMIO控制速度:TB6612
    ENCODER_MOTOR_2_PWM_MODE,     // 2PWMIO控制方向和速度:AT8236
}encoder_motor_mode_e;

typedef struct {
    encoder_motor_mode_e mode;
    /* 物理属性 */
    float ratio;
    uint32_t ppr;

    /* io配置 */
    uint8_t enc_a_io,enc_b_io;

    uint8_t dir_1_io,dir_2_io,pwm_io;
    uint8_t pwm_channel;

    uint8_t pwm_1_io,pwm_2_io;
    uint8_t pwm_1_channel,pwm_2_channel;

    /* 编码器软件正交解码状态（供中断服务函数使用） */
    volatile uint8_t enc_last_state;

    volatile long pulseCount;
    long lastCount;
    unsigned long lastTime;

    float rpm;
}encoder_motor_t;

void encoder_motor_init(encoder_motor_t *motor);
void encoder_motor_update(encoder_motor_t *motor);
void encoder_motor_set_compare(encoder_motor_t *motor, int16_t compare);

#ifdef __cplusplus
}
#endif

#endif

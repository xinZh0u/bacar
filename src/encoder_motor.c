#include "encoder_motor.h"

#define PWM_FREQ 1000
#define PWM_RESOLUTION 10   // 1024

/*
 * ESP32-C3 没有 PCNT 外设，这里使用 GPIO 中断 + 软件正交解码。
 * 状态 = (A << 1) | B，索引 = (上一次状态 << 2) | 当前状态。
 */
static const int8_t ENCODER_DIR_TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
};

static void IRAM_ATTR encoder_motor_isr(void *arg) {
    encoder_motor_t *motor = (encoder_motor_t *)arg;
    uint8_t state = (uint8_t)((digitalRead(motor->enc_a_io) << 1) |
                               digitalRead(motor->enc_b_io));
    motor->pulseCount += ENCODER_DIR_TABLE[(motor->enc_last_state << 2) | state];
    motor->enc_last_state = state;
}

void encoder_motor_init(encoder_motor_t *motor) {
    /* 编码器初始化 */
    pinMode(motor->enc_a_io, INPUT_PULLUP);
    pinMode(motor->enc_b_io, INPUT_PULLUP);

    motor->pulseCount = 0;
    motor->lastCount = 0;
    motor->lastTime = millis();
    motor->enc_last_state = (uint8_t)((digitalRead(motor->enc_a_io) << 1) |
                                      digitalRead(motor->enc_b_io));

    attachInterruptArg(digitalPinToInterrupt(motor->enc_a_io),
                       encoder_motor_isr, motor, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(motor->enc_b_io),
                       encoder_motor_isr, motor, CHANGE);

    /* 电机初始化 */
    if(motor->mode == ENCODER_MOTOR_1_PWM_MODE) {
        pinMode(motor->dir_1_io, OUTPUT);
        pinMode(motor->dir_2_io, OUTPUT);
        pinMode(motor->pwm_io, OUTPUT);
        ledcSetup(motor->pwm_channel, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(motor->pwm_io, motor->pwm_channel);     
    } 
    else if(motor->mode == ENCODER_MOTOR_2_PWM_MODE) {
        pinMode(motor->pwm_1_io, OUTPUT);
        pinMode(motor->pwm_2_io, OUTPUT);
        
        ledcSetup(motor->pwm_1_channel, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(motor->pwm_1_io, motor->pwm_1_channel);
        ledcSetup(motor->pwm_2_channel, PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(motor->pwm_2_io, motor->pwm_2_channel);
    }
}

void encoder_motor_update(encoder_motor_t *motor) {
    unsigned long dt = millis() - motor->lastTime;
    if (dt >= 10) {
        long deltaCount = motor->pulseCount - motor->lastCount;
        motor->lastCount = motor->pulseCount;
        motor->lastTime = millis();
        motor->rpm = (float)deltaCount / motor->ppr / motor->ratio * (60000.0 / dt) / 4.0f;
    }
}

void encoder_motor_set_compare(encoder_motor_t *motor, int16_t compare) {
    if(compare>1023)compare = 1023;
    if(compare<-1023)compare = -1023;
    if(motor->mode == ENCODER_MOTOR_1_PWM_MODE) {
        ledcWrite(motor->pwm_channel, compare >= 0 ? compare : -compare);
        if(compare>0) {
            digitalWrite(motor->dir_1_io,LOW);
            digitalWrite(motor->dir_2_io,HIGH);
        }
        else {
            digitalWrite(motor->dir_1_io,HIGH);
            digitalWrite(motor->dir_2_io,LOW);
        }
    }
    else if(motor->mode == ENCODER_MOTOR_2_PWM_MODE) {
        if(compare>0) {
            ledcWrite(motor->pwm_1_channel,compare);
            ledcWrite(motor->pwm_2_channel,0);
        }
        else {
            ledcWrite(motor->pwm_1_channel,0);
            ledcWrite(motor->pwm_2_channel,-compare);
        }
    }
}

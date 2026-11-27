#ifndef PID_H
#define PID_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float kp;
    float ki;
    float kd;

    float max_out;
    float integral_limit;
    float deadband;

    float target;
    float actual;
    float error;
    float last_error;
    float last_actual;
    bool has_last_actual;
    float integral;
    float output;

    int64_t last_time_us;
} pid_ctrl_t;

void pid_struct_init(pid_ctrl_t *pid, float maxout, float integral_limit,
                     float kp, float ki, float kd, float deadband);

/**
 * @brief 计算PID输出
 * @note  dt 由 pid 内部记录的上一次调用时间与当前系统时间自动计算
 */
float pid_calc(pid_ctrl_t *pid, float get, float set);

#ifdef __cplusplus
}
#endif

#endif

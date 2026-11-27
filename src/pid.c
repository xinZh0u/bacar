#include "pid.h"

#include <math.h>
#include <stdbool.h>
#include "esp_timer.h"

static float limit(float value, float max)
{
    if (value > max)
        return max;
    if (value < -max)
        return -max;
    return value;
}

void pid_struct_init(pid_ctrl_t *pid, float maxout, float integral_limit,
                     float kp, float ki, float kd, float deadband)
{
    if (pid == 0)
        return;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->max_out = maxout;
    pid->integral_limit = integral_limit;
    pid->deadband = deadband;

    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->last_actual = 0.0f;
    pid->has_last_actual = false;
    pid->integral = 0.0f;
    pid->output = 0.0f;
    pid->last_time_us = 0;
}

float pid_calc(pid_ctrl_t *pid, float get, float set)
{
    if (pid == 0)
        return 0.0f;

    pid->target = set;
    pid->actual = get;

    int64_t now_us = esp_timer_get_time();
    float dt = 0.0f;
    if (pid->last_time_us != 0)
        dt = (now_us - pid->last_time_us) / 1000000.0f;
    pid->last_time_us = now_us;

    float error = set - get;
    pid->error = (fabsf(error) < pid->deadband) ? 0.0f : error;

    float p = pid->kp * pid->error;
    float d = 0.0f;

    if (dt > 0.0f)
    {
        /* 抗积分饱和：输出已饱和且误差继续让输出增大的方向时，停止积分 */
        // bool saturated_pos = (pid->output >= pid->max_out) && (pid->error > 0.0f);
        // bool saturated_neg = (pid->output <= -pid->max_out) && (pid->error < 0.0f);
        // if (!saturated_pos && !saturated_neg)
        // {
            pid->integral += pid->error * dt;
            pid->integral = limit(pid->integral, pid->integral_limit);
        // }

        /* 微分作用在测量值上，避免设定值突变/死区切换造成微分冲击 */
        if (pid->has_last_actual)
        {
            d = -pid->kd * (get - pid->last_actual) / dt;
        }
    }

    float i = pid->ki * pid->integral;

    pid->last_error = pid->error;
    pid->last_actual = get;
    pid->has_last_actual = true;
    pid->output = limit(p + i + d, pid->max_out);

    return pid->output;
}

#include "ramp.h"

#include <stdbool.h>

float data_limit(float data, float max, float min) {
    if(data>max) data = max;
    if(data<min) data = min;
    return data;
}

void ramp_init(ramp_t *ramp, float frame_period_up,float frame_period_down, float min, float max){
    ramp->frame_period = frame_period_up;
	ramp->frame_period_down = frame_period_down;
    ramp->min = min;
    ramp->max = max;
    ramp->out = 0;
}

float ramp_calc(ramp_t *ramp, float target) {
    ramp->target = target;

    float error = ramp->target - ramp->out;
    
    // 如果误差极小，直接等于目标，避免浮点震荡
    if (fabsf(error) < 1e-6f) {
        ramp->out = ramp->target;
    } 
    else {
        float step;
        
        bool is_same_sign = (ramp->out * ramp->target >= 0);
        bool is_decelerating = false;

        if (is_same_sign) {
            // 同号时，如果当前值的绝对值大于目标值的绝对值，说明在往回走（减速）
            if (fabsf(ramp->out) > fabsf(ramp->target)) {
                is_decelerating = true;
            }
        } else {
            // 异号时，通常视为需要先减速到0。
            // 为了平滑，异号阶段通常也使用较小的减速斜率，或者统一使用减速斜率直到过零
            is_decelerating = true; 
        }

        // 选择步长
        step = is_decelerating ? ramp->frame_period_down : ramp->frame_period;

        // 执行步进
        if (error > 0) {
            // 需要增加
            ramp->out += step;
            // 防止过冲
            if (ramp->out > ramp->target) {
                ramp->out = ramp->target;
            }
        } else {
            // 需要减小
            ramp->out -= step;
            // 防止过冲
            if (ramp->out < ramp->target) {
                ramp->out = ramp->target;
            }
        }
    }

	ramp->out = data_limit(ramp->out, ramp->max, ramp->min);
    return ramp->out;
}
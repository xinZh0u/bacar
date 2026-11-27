#ifndef RAMP_H
#define RAMP_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float target;
    float out;
    float min, max;
    float frame_period;
	float frame_period_down;
} ramp_t;

float data_limit(float data, float max, float min);

void ramp_init(ramp_t *ramp, float frame_period_up,float frame_period_down, float min, float max);

float ramp_calc(ramp_t *ramp, float target);

#ifdef __cplusplus
}
#endif

#endif

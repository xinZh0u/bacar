#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 电池 ADC 配置（可在 platformio.ini 用 -D 覆盖）
 *
 * 电池+ --- 100K --- ADC --- 22K --- GND
 */
#ifndef BATTERY_ADC_PIN
#define BATTERY_ADC_PIN         1
#endif
#ifndef BATTERY_ADC_SAMPLES
#define BATTERY_ADC_SAMPLES     16
#endif

/* 3S 锂电池：满电 12.6V，低点 10.5V */
#ifndef BATTERY_FULL_MV
#define BATTERY_FULL_MV         12600
#endif
#ifndef BATTERY_EMPTY_MV
#define BATTERY_EMPTY_MV        10500
#endif

/* 分压电阻 (kΩ)：上 100K，下 22K */
#ifndef BATTERY_DIV_TOP_K
#define BATTERY_DIV_TOP_K       100.0f
#endif
#ifndef BATTERY_DIV_BOTTOM_K
#define BATTERY_DIV_BOTTOM_K    22.0f
#endif

/* 低于该电压视为未接电池/读数无效，界面显示 -- */
#ifndef BATTERY_MIN_VALID_MV
#define BATTERY_MIN_VALID_MV    6000
#endif

void battery_init(void);
void battery_update(void);

/* 返回 0~100；-1 表示未接电池/读数无效 */
int battery_percent(void);

/* 返回电池电压 (V)；0 表示未知 */
float battery_voltage(void);

#ifdef __cplusplus
}
#endif

#endif

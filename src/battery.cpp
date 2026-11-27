#include "battery.h"

#include <Arduino.h>

static int s_percent = -1;
static float s_voltage = 0.0f;

static void battery_task(void *args)
{
    (void)args;
    for (;;) {
        battery_update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void battery_update(void)
{
    uint32_t sum_mv = 0;

    for (int i = 0; i < BATTERY_ADC_SAMPLES; i++) {
        sum_mv += analogReadMilliVolts(BATTERY_ADC_PIN);
        delayMicroseconds(200);
    }

    float pin_mv = (float)sum_mv / (float)BATTERY_ADC_SAMPLES;

    /* 分压还原：Vbat = Vpin * (Rtop + Rbottom) / Rbottom */
    float bat_mv = pin_mv * (BATTERY_DIV_TOP_K + BATTERY_DIV_BOTTOM_K)
                           / BATTERY_DIV_BOTTOM_K;

    s_voltage = bat_mv / 1000.0f;

    if (bat_mv < BATTERY_MIN_VALID_MV) {
        s_percent = -1;
        return;
    }

    if (bat_mv >= BATTERY_FULL_MV) {
        s_percent = 100;
    }
    else if (bat_mv <= BATTERY_EMPTY_MV) {
        s_percent = 0;
    }
    else {
        s_percent = (int)((bat_mv - BATTERY_EMPTY_MV) * 100.0f
                          / (BATTERY_FULL_MV - BATTERY_EMPTY_MV) + 0.5f);
    }
}

void battery_init(void)
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    battery_update();
    Serial.printf("[BATT] ADC pin=%d, %.2fV, %d%%\n",
                  BATTERY_ADC_PIN, s_voltage, s_percent);

    xTaskCreate(battery_task, "battery_task", 3072, NULL, 4, NULL);
}

int battery_percent(void)
{
    return s_percent;
}

float battery_voltage(void)
{
    return s_voltage;
}

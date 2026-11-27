#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "chassis.h"
#include "web_server.h"
#include "web_control.h"
#include "control.h"
#include "imu.h"
#include "mp3_player.h"
#include "battery.h"

void setup() {
    Serial.begin(115200);
    battery_init();
    mp3_player_init();
    chassis_init();
    imu_init();
    control_init();

    web_server_init();
}

int mm = 2;

void loop() {
    //web_control_debug();
    // Serial.printf("motor:%f,%f,%f,%f\n",chassis.motors[0].spd_get,chassis.motors[1].spd_get,chassis.motors[2].spd_get,chassis.motors[3].spd_get);

    // Serial.printf("pid:%f,%f,%f,%f,",chassis.motors[mm].spd_set,chassis.motors[mm].spd_get,chassis.motors[mm].spd_pid.output,chassis.motors[mm].spd_pid.integral);        
    // Serial.printf("%f,%f,%f,%f,%f,%f\n",chassis.spd_set.vx,chassis.spd_get.vx,
    //     chassis.spd_set.vy,chassis.spd_get.vy,
    //     chassis.spd_set.vw,chassis.spd_get.vw);

    // Serial.printf("spd:%f,%f,%f,%f,%f,%f\n",control.spd_output.vx,control.spd_output.vy,control.spd_output.vw,
    // control.spd_input.vx,control.spd_input.vy,control.spd_input.vw);

    Serial.printf("imu:%f,%f,%f,%f\n",imu.roll,imu.pitch,imu.yaw,control.err_yaw);

    vTaskDelay(pdMS_TO_TICKS(5));
}

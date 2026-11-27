#ifndef IMU_H
#define IMU_H

#include <stdint.h>

/*
 * IMU 型号选择（二选一）：
 *   1. 直接取消注释下面的型号宏；
 *   2. 或在 platformio.ini 的 build_flags 中加入 -D IMU_TYPE_MPU6500。
 * 两个都不定义时默认使用 MPU6500。
 */
#define IMU_TYPE_MPU6500
// #define IMU_TYPE_MPU9250

#if !defined(IMU_TYPE_MPU6500) && !defined(IMU_TYPE_MPU9250)
#define IMU_TYPE_MPU6500
#endif

#if defined(IMU_TYPE_MPU6500) && defined(IMU_TYPE_MPU9250)
#error "IMU_TYPE_MPU6500 与 IMU_TYPE_MPU9250 只能定义一个"
#endif

/* I2C 引脚与从机地址 */
#define IMU_SCL     9
#define IMU_SDA     10
#define IMU_ADDR    0x68

typedef struct {
    float ax,ay,az;
    float wx,wy,wz;
    float roll,pitch,yaw;
    float resultantG;
    float temp;

    uint8_t online;
}imu_t;

extern imu_t imu;

void imu_init();

#endif

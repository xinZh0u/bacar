#include "imu.h"

#if defined(IMU_TYPE_MPU9250)
#include <MPU9250_WE.h>
#define IMU_BASE_CLASS MPU9250_WE
#define IMU_MODEL_NAME "MPU9250"
#else
#include <MPU6500_WE.h>
#define IMU_BASE_CLASS MPU6500_WE
#define IMU_MODEL_NAME "MPU6500"
#endif

#include <Wire.h>
#include <math.h>

imu_t imu;

/*
 * 兼容 MPU6500(WHO_AM_I=0x70) / MPU9250(WHO_AM_I=0x71)：
 * 很多标称 MPU9250 的模块实际主芯片是 MPU6500，严格按型号校验会初始化失败。
 * init(expected) 是 protected，所以在派生类里手动按两种 ID 依次尝试。
 */
class ImuDevice : public IMU_BASE_CLASS {
public:
    explicit ImuDevice(uint8_t addr) : IMU_BASE_CLASS(addr) {}

    bool init_auto() {
#if defined(IMU_TYPE_MPU9250)
        if (MPU6500_WE::init(0x71)) {
            return true;
        }
        return MPU6500_WE::init(0x70);
#else
        if (MPU6500_WE::init(0x70)) {
            return true;
        }
        return MPU6500_WE::init(0x71);
#endif
    }
};

ImuDevice myIMU(IMU_ADDR);

/* 加速度计修正强度（1/s），越大收敛越快但越容易被振动干扰 */
#define IMU_ACC_KP          2.0f

/* ---------------- yaw 卡尔曼滤波参数 ---------------- */
/* 状态：[yaw(deg), 陀螺 z 轴零偏(deg/s)] */
#define YAW_KF_Q_ANGLE      0.01f    /* yaw 过程噪声 (deg^2/s)，反映陀螺噪声 */
#define YAW_KF_Q_BIAS       0.0001f  /* 零偏随机游走 (deg^2/s^3) */
#define YAW_KF_R_ZERO_RATE  0.05f    /* 零角速度观测噪声 (deg/s)^2 */

/* 静止检测阈值：加速度模长接近 1g，且三轴角速度都很小 */
#define IMU_STILL_ACC_TOL   0.12f    /* g */
#define IMU_STILL_GYRO_DPS  2.0f     /* deg/s */
#define IMU_STILL_FRAMES    30       /* 连续 30 帧（约 300ms）才认为静止 */

/* 姿态四元数（机体坐标系 -> 世界坐标系） */
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

/* yaw 卡尔曼滤波器 */
typedef struct {
    float yaw;              /* deg，连续角度 */
    float bias;             /* deg/s */
    float p00, p01, p11;    /* 协方差 */
} yaw_kf_t;

static yaw_kf_t yaw_kf = {0.0f, 0.0f, 1.0f, 0.0f, 0.25f};

/* 预测：yaw += (wz - bias) * dt */
static void yaw_kf_predict(yaw_kf_t *kf, float rate, float dt) {
    kf->yaw += (rate - kf->bias) * dt;

    /* P = F P F^T + Q，F = [[1, -dt], [0, 1]] */
    float p00 = kf->p00 - 2.0f * dt * kf->p01 + dt * dt * kf->p11
                + YAW_KF_Q_ANGLE * dt;
    float p01 = kf->p01 - dt * kf->p11;
    float p11 = kf->p11 + YAW_KF_Q_BIAS * dt;

    kf->p00 = p00;
    kf->p01 = p01;
    kf->p11 = p11;
}

/*
 * 零角速度更新：静止时真实角速度为 0，测得 rate 应等于零偏。
 * 观测模型 z = rate，H = [0, 1]，预测观测 = bias。
 * 这样既能修正零偏，也能修正与零偏相关的 yaw 漂移。
 */
static void yaw_kf_zero_rate_update(yaw_kf_t *kf, float rate) {
    float s = kf->p11 + YAW_KF_R_ZERO_RATE;
    if (s <= 0.0f) {
        return;
    }

    float innov = rate - kf->bias;
    float k0 = kf->p01 / s;
    float k1 = kf->p11 / s;

    float p01_old = kf->p01;

    kf->yaw  += k0 * innov;
    kf->bias += k1 * innov;

    /* 限制零偏估计范围，避免误判静止把 yaw 拉飞 */
    if (kf->bias >  5.0f) kf->bias =  5.0f;
    if (kf->bias < -5.0f) kf->bias = -5.0f;

    kf->p00 -= k0 * p01_old;
    kf->p01 -= k0 * kf->p11;
    kf->p11 -= k1 * kf->p11;
}

/* 把角度限制到 [-180, 180] */
static float wrap_deg(float angle) {
    while(angle > 180.0f)  angle -= 360.0f;
    while(angle < -180.0f) angle += 360.0f;
    return angle;
}

/* 把 yaw 限制到 [0, 360) */
static float wrap_yaw_360(float angle) {
    while(angle >= 360.0f) angle -= 360.0f;
    while(angle < 0.0f)    angle += 360.0f;
    return angle;
}

/* 用加速度计初始化四元数（yaw 置 0），翻过来也能正确初始化 */
static void quat_init_from_accel(float ax, float ay, float az) {
    float roll  = atan2f(ay, az);
    float pitch = atan2f(ax, sqrtf(ay * ay + az * az));

    float cr = cosf(roll * 0.5f),  sr = sinf(roll * 0.5f);
    float cp = cosf(pitch * 0.5f), sp = sinf(pitch * 0.5f);

    q0 = cr * cp;
    q1 = sr * cp;
    q2 = cr * sp;
    q3 = -sr * sp;

    float norm = sqrtf(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    q0 /= norm; q1 /= norm; q2 /= norm; q3 /= norm;
}

/* 四元数姿态更新：陀螺仪积分 + 加速度计重力方向修正 */
static void quat_update(float wx, float wy, float wz,
                        float ax, float ay, float az, float dt) {
    /* 角速度转 rad/s */
    float gx = wx * DEG_TO_RAD;
    float gy = wy * DEG_TO_RAD;
    float gz = wz * DEG_TO_RAD;

    /* 加速度计归一化 */
    float an = sqrtf(ax * ax + ay * ay + az * az);
    if(an > 0.1f) {
        ax /= an; ay /= an; az /= an;

        /* 当前四元数估计出的重力方向（机体系） */
        float vx = 2.0f * (q1 * q3 - q0 * q2);
        float vy = 2.0f * (q0 * q1 + q2 * q3);
        float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        /* 测量重力与估计重力的叉积，只修正 roll/pitch，不影响 yaw */
        float ex = ay * vz - az * vy;
        float ey = az * vx - ax * vz;
        float ez = ax * vy - ay * vx;

        gx += IMU_ACC_KP * ex;
        gy += IMU_ACC_KP * ey;
        gz += IMU_ACC_KP * ez;
    }

    /* 一阶龙格库塔积分 */
    float dq0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float dq1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float dq2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float dq3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    q0 += dq0 * dt;
    q1 += dq1 * dt;
    q2 += dq2 * dt;
    q3 += dq3 * dt;

    float norm = sqrtf(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    q0 /= norm; q1 /= norm; q2 /= norm; q3 /= norm;
}

/* 四元数转欧拉角（弧度，ZYX 顺序） */
static void quat_to_euler(float *roll, float *pitch, float *yaw) {
    *roll  = atan2f(2.0f * (q0 * q1 + q2 * q3),
                    1.0f - 2.0f * (q1 * q1 + q2 * q2));

    float sin_pitch = 2.0f * (q0 * q2 - q3 * q1);
    if(sin_pitch > 1.0f)  sin_pitch = 1.0f;
    if(sin_pitch < -1.0f) sin_pitch = -1.0f;
    *pitch = asinf(sin_pitch);

    *yaw = atan2f(2.0f * (q0 * q3 + q1 * q2),
                  1.0f - 2.0f * (q2 * q2 + q3 * q3));
}

void imu_task(void * args) {
    TickType_t wake_time = xTaskGetTickCount();
    const TickType_t delay_time = pdMS_TO_TICKS(10);

    uint32_t last_us = micros();
    uint8_t first_run = 1;
    uint16_t still_frames = 0;

    for(;;) {
        xyzFloat gValue = myIMU.getGValues();
        imu.ax = gValue.x;
        imu.ay = gValue.y;
        imu.az = gValue.z;

        xyzFloat gyr = myIMU.getGyrValues();
        imu.wx = gyr.x;
        imu.wy = gyr.y;
        imu.wz = gyr.z;

        /* 采样周期，异常时退回到 10ms */
        uint32_t now_us = micros();
        float dt = (now_us - last_us) * 1e-6f;
        last_us = now_us;
        if(dt <= 0.0f || dt > 0.5f) dt = 0.01f;

        if(first_run) {
            /* 首帧用加速度计初始化姿态，避免启动时角度跳变 */
            quat_init_from_accel(imu.ax, imu.ay, imu.az);
            yaw_kf.yaw  = 0.0f;
            yaw_kf.bias = 0.0f;
            yaw_kf.p00  = 1.0f;
            yaw_kf.p01  = 0.0f;
            yaw_kf.p11  = 0.25f;
            first_run = 0;
        }
        else {
            quat_update(imu.wx, imu.wy, imu.wz,
                        imu.ax, imu.ay, imu.az, dt);
        }

        /* yaw 卡尔曼：先用陀螺预测 */
        yaw_kf_predict(&yaw_kf, imu.wz, dt);

        /* 静止检测：加速度模长接近 1g 且三轴角速度都很小 */
        float an = sqrtf(imu.ax * imu.ax + imu.ay * imu.ay + imu.az * imu.az);
        float wnorm = sqrtf(imu.wx * imu.wx + imu.wy * imu.wy + imu.wz * imu.wz);
        bool still = (fabsf(an - 1.0f) < IMU_STILL_ACC_TOL) &&
                     (wnorm < IMU_STILL_GYRO_DPS);
        if (still) {
            if (still_frames < IMU_STILL_FRAMES) {
                still_frames++;
            }
        }
        else {
            still_frames = 0;
        }

        /* 连续静止一段时间后，用零角速度观测修正零偏和 yaw */
        if (still_frames >= IMU_STILL_FRAMES) {
            yaw_kf_zero_rate_update(&yaw_kf, imu.wz);
        }

        float roll_rad, pitch_rad, quat_yaw_rad;
        quat_to_euler(&roll_rad, &pitch_rad, &quat_yaw_rad);
        (void)quat_yaw_rad;

        /* 保持原有方向约定：pitch 取反；
           roll/pitch 输出 [-180,180]，yaw 输出 [0,360) */
        imu.roll  = wrap_deg(roll_rad * RAD_TO_DEG);
        imu.pitch = wrap_deg(-pitch_rad * RAD_TO_DEG);
        imu.yaw   = wrap_yaw_360(yaw_kf.yaw);

        imu.temp = myIMU.getTemperature();
        imu.resultantG = myIMU.getResultantG(gValue);

        vTaskDelayUntil(&wake_time, delay_time);
    }
}

void imu_init() {
    Wire.begin(IMU_SDA,IMU_SCL);

    uint8_t who = myIMU.whoAmI();
    Serial.printf("[IMU] target: %s, WHO_AM_I=0x%02X (%s)\n",
                  IMU_MODEL_NAME, who,
                  who == 0x71 ? "MPU9250" :
                  (who == 0x70 ? "MPU6500" : "unknown"));

    if(!myIMU.init_auto()){
        Serial.printf("[IMU] %s does not respond\n", IMU_MODEL_NAME);
        imu.online = 0;
    }
    else{
        Serial.printf("[IMU] connected (target %s, WHO_AM_I=0x%02X)\n",
                      IMU_MODEL_NAME, who);
        imu.online = 1;
    }

    Serial.printf("[IMU] Position your %s flat and don't move it - calibrating...\n", IMU_MODEL_NAME);
    delay(1000);
    myIMU.autoOffsets();
    Serial.println("[IMU] Done!");

    myIMU.enableGyrDLPF();
    myIMU.setGyrDLPF(MPU6500_DLPF_6);
    myIMU.setSampleRateDivider(5);
    myIMU.setGyrRange(MPU6500_GYRO_RANGE_1000);
    myIMU.setAccRange(MPU6500_ACC_RANGE_2G);
    myIMU.enableAccDLPF(true);
    myIMU.setAccDLPF(MPU6500_DLPF_6);
    delay(200);

#if defined(IMU_TYPE_MPU9250)
    /* 磁力计检测：AK8963 的 WHO_AM_I 应为 0x48 */
    if (imu.online && myIMU.initMagnetometer()) {
        Serial.printf("[IMU] magnetometer: AK8963 detected (WHO_AM_I=0x%02X)\n",
                      myIMU.whoAmIMag());
    } else {
        Serial.println("[IMU] magnetometer: not detected");
    }
#endif

    xTaskCreate(imu_task, "imu_task", 1024 * 4,(void* ) 0, 10, NULL);
}

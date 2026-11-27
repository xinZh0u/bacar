#include "control.h"
#include "web_control.h"
#include "chassis.h"
#include "imu.h"
#include <math.h>

control_t control;

void mode_switch() {

    
    if(web_control.circle && imu.online && !control.rotate_able) {
        control.rotate_able = 1;
        control.rotate_speed = 500;
        control.forward_yaw = imu.yaw;
    }
    if(web_control.cross) {
        control.rotate_able = 0;    
    }
}

/* 平动限速旋转参数（线性缩放，力控/速控通用） */
#define CHASSIS_VXY_LIMIT_START_RPM   0.0f      // 平动速度低于此值，旋转不减速
#define CHASSIS_VXY_LIMIT_END_RPM     300.0f    // 平动速度高于此值，旋转缩放到最低
#define CHASSIS_VW_SCALE_MIN          0.5f      // 旋转速度最低缩放系数
#define CHASSIS_VW_SCALE_SLOPE        ((CHASSIS_VW_SCALE_MIN - 1.0f) / (CHASSIS_VXY_LIMIT_END_RPM - CHASSIS_VXY_LIMIT_START_RPM))

// float circle_error(float set,float get,float circle_para)
// {
//     float error;
//     if(set > get)
//     {
//         if(set - get> circle_para/2)
//             error = set - get - circle_para;
//         else
//             error = set - get;
//     }
//     else if(set < get)
//     {
//         if(set - get<-1*circle_para/2)
//             error = set - get +circle_para;
//         else
//             error = set - get;
//     }
//     else	error = 0;

//     return error;
// }

static void chassis_rotate_speed_down(void) {
	float vxy_max = fabs(control.spd_input.vx) > fabs(control.spd_input.vy) ? 
		fabs(control.spd_input.vx) : fabs(control.spd_input.vy);
	if (vxy_max > CHASSIS_VXY_LIMIT_START_RPM) {
		float vw_scale;
		if (vxy_max >= CHASSIS_VXY_LIMIT_END_RPM) {
			vw_scale = CHASSIS_VW_SCALE_MIN;
		} else {
			vw_scale = 1.0f + CHASSIS_VW_SCALE_SLOPE * (vxy_max - CHASSIS_VXY_LIMIT_START_RPM);
			if (vw_scale < CHASSIS_VW_SCALE_MIN)
				vw_scale = CHASSIS_VW_SCALE_MIN;
		}
		control.spd_input.vw *= vw_scale;
	}
}

void remote_control() {
    float vx,vy,vw;
    vx = web_control.ly;
    vy = web_control.lx;
    vw = web_control.rx;

    if(web_control.up)vx = 100;
    if(web_control.down)vx = -100;
    if(web_control.left)vy = -100;
    if(web_control.right)vy = 100;

    /* imu姿态获取 */
    control.err_yaw = control.forward_yaw - imu.yaw;
    control.err_yaw = fmodf(control.err_yaw,360.0f);
    if(control.err_yaw < 0)control.err_yaw += 360;
    float yaw_rad = control.err_yaw*PI/180.0f;

    if(control.rotate_able) {
        /* 先按普通模式得到世界坐标系速度，再旋转到当前车体坐标系。
           两个分量必须使用同一套比例系数，否则 err_yaw=0 时方向会反 180°。 */
        float vx_world = vx * PAD_LX_SCALE;
        float vy_world = vy * PAD_LY_SCALE;
        control.spd_input.vx =  vx_world * cos(yaw_rad) + vy_world * sin(yaw_rad);
        control.spd_input.vy = -vx_world * sin(yaw_rad) + vy_world * cos(yaw_rad);
        control.spd_input.vw = control.rotate_speed;
        chassis_rotate_speed_down();
    }
    else {
        control.spd_input.vx = vx * PAD_LX_SCALE;
        control.spd_input.vy = vy * PAD_LY_SCALE;
        control.spd_input.vw = vw * PAD_RX_SCALE;
    }

    /* 速度斜坡 */
    control.spd_output.vx = ramp_calc(&control.vx_ramp,control.spd_input.vx);
    control.spd_output.vy = ramp_calc(&control.vy_ramp,control.spd_input.vy);
    control.spd_output.vw = control.spd_input.vw;

    chassis.spd_set.vx = control.spd_output.vx;
    chassis.spd_set.vy = control.spd_output.vy;
    chassis.spd_set.vw = control.spd_output.vw;
}

/* ---------------- 小陀螺自动测试 ---------------- */
/*
 * 按下圆圈后循环执行：
 *   旋转 TEST_SPIN_MS -> 转回 control.forward_yaw -> 静止等待 TEST_WAIT_MS
 * 按下叉号停止（由 mode_switch 清除 rotate_able）。
 */
typedef enum {
    TEST_IDLE = 0,
    TEST_SPIN,
    TEST_RETURN,
    TEST_WAIT,
} test_phase_t;

#define TEST_SPIN_MS        5000    /* 自转时间 */
#define TEST_WAIT_MS        5000    /* 回正后等待时间 */
#define TEST_RETURN_MS      5000    /* 回正超时 */
#define TEST_RETURN_KP      3.0f    /* 回正 P 增益 (rpm/deg) */
#define TEST_RETURN_MAX     300.0f  /* 回正最大转速 (rpm) */
#define TEST_RETURN_TOL     1.5f    /* 回正到位误差 (deg) */
#define TEST_DIR_SAMPLE_MS  300     /* 自转稳定后采样实际转向 */

static test_phase_t test_phase = TEST_IDLE;
static uint32_t test_phase_ms = 0;
static float test_yaw_dir = 1.0f;   /* d(imu.yaw)/d(vw) 的符号 */
static uint8_t test_dir_ready = 0;
static uint8_t test_armed_last = 0;

static float wrap180(float angle) {
    angle = fmodf(angle + 180.0f, 360.0f);
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    return angle - 180.0f;
}

static void test_set_phase(test_phase_t phase) {
    test_phase = phase;
    test_phase_ms = millis();

    switch (phase) {
        case TEST_SPIN:
            Serial.printf("[TEST] spin: forward_yaw=%.1f yaw=%.1f\n",
                          control.forward_yaw, imu.yaw);
            break;
        case TEST_RETURN:
            Serial.printf("[TEST] return: target=%.1f yaw=%.1f dir=%.0f\n",
                          control.forward_yaw, imu.yaw, test_yaw_dir);
            break;
        case TEST_WAIT:
            Serial.printf("[TEST] wait: yaw=%.1f\n", imu.yaw);
            break;
        default:
            break;
    }
}

void remote_control_test() {
    uint32_t now = millis();
    float vw = 0.0f;

    /* 未使能（未按圆圈 / 已按叉号）：停机 */
    if (!control.rotate_able) {
        if (test_armed_last) {
            Serial.println("[TEST] stop");
        }
        test_armed_last = 0;
        test_phase = TEST_IDLE;
        test_dir_ready = 0;
    }
    else {
        /* 圆圈上升沿：从头开始 */
        if (!test_armed_last) {
            test_armed_last = 1;
            test_yaw_dir = 1.0f;
            test_dir_ready = 0;
            test_set_phase(TEST_SPIN);
        }

        switch (test_phase) {
            case TEST_SPIN:
                vw = control.rotate_speed;

                /* 自转稳定后采样一次实际转向，回正时据此确定符号 */
                if (!test_dir_ready && (now - test_phase_ms) >= TEST_DIR_SAMPLE_MS) {
                    if (fabsf(imu.wz) > 1.0f) {
                        test_yaw_dir = (imu.wz > 0.0f) ? 1.0f : -1.0f;
                    }
                    test_dir_ready = 1;
                    Serial.printf("[TEST] wz=%.1f dir=%.0f\n", imu.wz, test_yaw_dir);
                }

                if ((now - test_phase_ms) >= TEST_SPIN_MS) {
                    test_set_phase(TEST_RETURN);
                }
                break;

            case TEST_RETURN: {
                float err = wrap180(control.forward_yaw - imu.yaw);
                float cmd = TEST_RETURN_KP * err;

                if (cmd >  TEST_RETURN_MAX) cmd =  TEST_RETURN_MAX;
                if (cmd < -TEST_RETURN_MAX) cmd = -TEST_RETURN_MAX;

                vw = test_yaw_dir * cmd;

                if (fabsf(err) < TEST_RETURN_TOL) {
                    test_set_phase(TEST_WAIT);
                }
                else if ((now - test_phase_ms) >= TEST_RETURN_MS) {
                    Serial.printf("[TEST] return timeout: err=%.1f\n", err);
                    test_set_phase(TEST_WAIT);
                }
                break;
            }

            case TEST_WAIT:
                vw = 0.0f;
                if ((now - test_phase_ms) >= TEST_WAIT_MS) {
                    test_set_phase(TEST_SPIN);
                }
                break;

            case TEST_IDLE:
            default:
                vw = 0.0f;
                break;
        }
    }

    /* 与 remote_control 共用输出通道 */
    control.spd_input.vx = 0.0f;
    control.spd_input.vy = 0.0f;
    control.spd_input.vw = vw;

    control.spd_output.vx = ramp_calc(&control.vx_ramp, control.spd_input.vx);
    control.spd_output.vy = ramp_calc(&control.vy_ramp, control.spd_input.vy);
    control.spd_output.vw = control.spd_input.vw;

    chassis.spd_set.vx = control.spd_output.vx;
    chassis.spd_set.vy = control.spd_output.vy;
    chassis.spd_set.vw = control.spd_output.vw;
}

void control_task(void * args) {
    TickType_t wake_time = xTaskGetTickCount();
    const TickType_t delay_time = pdMS_TO_TICKS(10);
    for(;;) {
        mode_switch();
        remote_control();
        // remote_control_test();

        vTaskDelayUntil(&wake_time, delay_time);
    }
}

void control_init() {
    ramp_init(&control.vx_ramp,10,30,-530,530);
    ramp_init(&control.vy_ramp,10,30,-530,530);
    xTaskCreate(control_task, "control_task", 1024 * 4,(void* ) 0, 10, NULL);
}

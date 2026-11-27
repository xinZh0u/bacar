#ifndef WEB_CONTROL_H
#define WEB_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PS3 布局手柄数据
 *
 * 摇杆范围：-100 ~ 100
 *   lx / rx：向右为正
 *   ly / ry：向上为正
 *
 * 按键：0 = 松开，1 = 按下
 */
typedef struct {
    /* 左摇杆 / 右摇杆 */
    int8_t lx;
    int8_t ly;
    int8_t rx;
    int8_t ry;

    /* 十字方向键 */
    uint8_t up;
    uint8_t down;
    uint8_t left;
    uint8_t right;

    /* 右侧功能键 */
    uint8_t triangle;
    uint8_t circle;
    uint8_t cross;
    uint8_t square;

    /* 肩键 */
    uint8_t l1;
    uint8_t l2;
    uint8_t r1;
    uint8_t r2;

    /* 摇杆按下 */
    uint8_t l3;
    uint8_t r3;

    /* 中间功能键 */
    uint8_t select;
    uint8_t start;
    uint8_t ps;
} web_control_t;

/* 全局唯一手柄数据，外部可直接读取 */
extern web_control_t web_control;

/* 注册 /control 页面及相关路由 */
void web_control_register();

/* 通过串口打印当前手柄数据 */
void web_control_debug();

#ifdef __cplusplus
}
#endif

#endif

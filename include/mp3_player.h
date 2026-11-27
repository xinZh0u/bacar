#ifndef MP3_PLAYER_H
#define MP3_PLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DFPlayer Mini 串口配置（按实际接线修改）
 *   ESP32 RX  <- MP3 TX
 *   ESP32 TX  -> MP3 RX
 *   共地
 */
#ifndef MP3_UART_NUM
#define MP3_UART_NUM        1
#endif
#ifndef MP3_UART_RX_PIN
#define MP3_UART_RX_PIN     4
#endif
#ifndef MP3_UART_TX_PIN
#define MP3_UART_TX_PIN     5
#endif
#ifndef MP3_UART_BAUD
#define MP3_UART_BAUD       9600
#endif

/* DFPlayer 音量范围 0~30 */
#ifndef MP3_DEFAULT_VOLUME
#define MP3_DEFAULT_VOLUME  15
#endif

/*
 * 播放模式（SELECT 键循环切换）
 * 使用 DFPlayer 自带的单曲循环 / 全部循环 / 随机全部，
 * 不再依赖固定的曲目数量。
 */

/*
 * 1: 播放 MP3/0001.mp3、MP3/0002.mp3 ...（playMp3Folder）
 * 0: 播放根目录 0001.mp3、0002.mp3 ...（play）
 *
 * 注意：方案 C 依赖模块的“全部循环/随机全部”，
 * 部分固件的这两个命令只覆盖根目录，建议优先使用 0（根目录）。
 */
#ifndef MP3_USE_MP3_FOLDER
#define MP3_USE_MP3_FOLDER  1
#endif

/* 播放模式（SELECT 键循环切换） */
typedef enum {
    MP3_MODE_SINGLE = 0,    /* 单曲循环 */
    MP3_MODE_SEQUENCE = 1,  /* 顺序播放 */
    MP3_MODE_RANDOM = 2,    /* 随机播放 */
    MP3_MODE_COUNT = 3,
} mp3_mode_t;

/* 默认播放模式 */
#ifndef MP3_DEFAULT_MODE
#define MP3_DEFAULT_MODE    MP3_MODE_SINGLE
#endif

void mp3_player_init(void);

void mp3_player_play(void);
void mp3_player_pause(void);
void mp3_player_toggle(void);
void mp3_player_next(void);
void mp3_player_prev(void);
void mp3_player_select(uint32_t index);
void mp3_adjust_volume(int delta);

/* 当前音量 0~30 */
uint8_t mp3_player_volume(void);

/* 当前音量百分比 0~100 */
uint8_t mp3_player_volume_percent(void);

/* 是否正在播放 */
uint8_t mp3_player_is_playing(void);

/* 当前曲目编号（1 开始） */
uint16_t mp3_player_track(void);

/* 模块报告的音频文件总数；0 表示未知 */
uint16_t mp3_player_file_count(void);

/* 当前播放模式 */
uint8_t mp3_player_mode(void);

#ifdef __cplusplus
}
#endif

#endif

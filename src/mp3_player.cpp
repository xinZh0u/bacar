#include "mp3_player.h"
#include "web_control.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

static HardwareSerial mp3Serial(MP3_UART_NUM);
static DFRobotDFPlayerMini player;

static volatile uint16_t s_track = 1;
static volatile uint16_t s_file_count = 0;
static volatile uint8_t s_volume = MP3_DEFAULT_VOLUME;
static volatile uint8_t s_playing = 0;
static volatile uint8_t s_started = 0;
static volatile uint8_t s_ready = 0;
static volatile uint8_t s_mode = MP3_DEFAULT_MODE;

static const char *const s_mode_names[MP3_MODE_COUNT] = {
    "单曲循环",
    "顺序播放",
    "随机播放",
};

/*
 * 方案 C：使用 DFPlayer 自带的播放模式，不依赖曲目总数。
 *   单曲循环 -> enableLoop
 *   顺序播放 -> enableLoopAll
 *   随机播放 -> randomAll
 */
static void apply_mode(void)
{
    if (!s_ready) {
        return;
    }

    switch (s_mode) {
        case MP3_MODE_SINGLE:
            player.disableLoopAll();
            player.enableLoop();
            break;

        case MP3_MODE_SEQUENCE:
            player.disableLoop();
            player.enableLoopAll();
            break;

        case MP3_MODE_RANDOM:
            player.disableLoop();
            player.disableLoopAll();
            if (s_started) {
                player.randomAll();
                if (!s_playing) {
                    player.pause();
                }
            }
            break;

        default:
            break;
    }
}

static void play_track(uint32_t index)
{
    if (!s_ready) {
        return;
    }

    if (index < 1) {
        index = 1;
    }
    else if (index > 65535u) {
        index = 65535u;
    }

    s_track = (uint16_t)index;

#if MP3_USE_MP3_FOLDER
    player.playMp3Folder((int)index);
#else
    player.play((int)index);
#endif

    s_playing = 1;
    s_started = 1;
}

void mp3_player_play(void)
{
    if (!s_ready) {
        return;
    }

    if (!s_started) {
        if (s_mode == MP3_MODE_RANDOM) {
            player.randomAll();
            s_started = 1;
            s_playing = 1;
        }
        else {
            play_track(s_track);
        }
        return;
    }

    player.start();
    s_playing = 1;
}

void mp3_player_pause(void)
{
    if (!s_ready) {
        return;
    }

    player.pause();
    s_playing = 0;
}

void mp3_player_toggle(void)
{
    if (s_playing) {
        mp3_player_pause();
    }
    else {
        mp3_player_play();
    }
}

void mp3_player_next(void)
{
    if (!s_ready) {
        return;
    }

    if (!s_started) {
        mp3_player_play();
        return;
    }

    player.next();
    s_playing = 1;
}

void mp3_player_prev(void)
{
    if (!s_ready) {
        return;
    }

    if (!s_started) {
        mp3_player_play();
        return;
    }

    player.previous();
    s_playing = 1;
}

void mp3_player_select(uint32_t index)
{
    play_track(index);
}

void mp3_adjust_volume(int delta)
{
    int v = (int)s_volume + delta;

    if (v < 0) {
        v = 0;
    }
    else if (v > 30) {
        v = 30;
    }

    s_volume = (uint8_t)v;

    if (s_ready) {
        player.volume(s_volume);
    }
}

uint8_t mp3_player_volume(void)
{
    return s_volume;
}

uint8_t mp3_player_volume_percent(void)
{
    return (uint8_t)(((uint16_t)s_volume * 100u) / 30u);
}

uint8_t mp3_player_is_playing(void)
{
    return s_playing;
}

uint16_t mp3_player_track(void)
{
    return s_track;
}

uint16_t mp3_player_file_count(void)
{
    return s_file_count;
}

uint8_t mp3_player_mode(void)
{
    return s_mode;
}

void mp3_player_task(void *args)
{
    (void)args;

    TickType_t wake_time = xTaskGetTickCount();
    const TickType_t delay_time = pdMS_TO_TICKS(10);

    uint8_t last_l1 = 0;
    uint8_t last_l2 = 0;
    uint8_t last_r1 = 0;
    uint8_t last_r2 = 0;
    uint8_t last_start = 0;
    uint8_t last_select = 0;

    uint32_t last_track_poll_ms = 0;

    mp3Serial.begin(MP3_UART_BAUD, SERIAL_8N1,
                    MP3_UART_RX_PIN, MP3_UART_TX_PIN);
    delay(300);

    if (player.begin(mp3Serial, true, true)) {
        s_ready = 1;
        player.volume(s_volume);
        apply_mode();

        /* 查询 TF 卡文件总数，用于界面显示“音频数量” */
        int count = player.readFileCounts(DFPLAYER_DEVICE_SD);
        if (count <= 0) {
            delay(200);
            count = player.readFileCounts(DFPLAYER_DEVICE_SD);
        }
        if (count > 0) {
            s_file_count = (uint16_t)count;
        }

        Serial.printf("[MP3] DFPlayer ready, volume=%d, files=%d, mode=%s\n",
                      s_volume, (int)s_file_count, s_mode_names[s_mode]);
    }
    else {
        Serial.println("[MP3] DFPlayer not found");
    }

    for (;;) {
        uint32_t now = millis();

        /* 处理 DFPlayer 的 ACK/状态消息，避免发送队列被堵住 */
        if (s_ready) {
            while (player.available()) {
                player.readType();
                player.read();
            }
        }

        /*
         * 用模块返回的当前文件号更新“第 n 首”。
         * 该查询会等待模块反馈，最坏阻塞约 500ms，所以放在 MP3 任务里，
         * 并且 1 秒才查询一次。
         */
        if (s_ready && (now - last_track_poll_ms) >= 1000) {
            last_track_poll_ms = now;

            int n = player.readCurrentFileNumber(DFPLAYER_DEVICE_SD);
            if (n > 0 && n <= 65535) {
                s_track = (uint16_t)n;
            }
        }

        /* 按键边沿检测：L1/L2 音量，R1/R2 上一首/下一首，
           START 播放/暂停，SELECT 切换播放模式 */
        uint8_t l1 = web_control.l1;
        uint8_t l2 = web_control.l2;
        uint8_t r1 = web_control.r1;
        uint8_t r2 = web_control.r2;
        uint8_t start = web_control.start;
        uint8_t select = web_control.select;

        if (l1 && !last_l1) {
            mp3_adjust_volume(+1);
        }
        if (l2 && !last_l2) {
            mp3_adjust_volume(-1);
        }
        if (r1 && !last_r1) {
            mp3_player_prev();
        }
        if (r2 && !last_r2) {
            mp3_player_next();
        }
        if (start && !last_start) {
            mp3_player_toggle();
        }
        if (select && !last_select) {
            s_mode = (uint8_t)((s_mode + 1) % MP3_MODE_COUNT);
            apply_mode();
            Serial.printf("[MP3] mode: %s\n", s_mode_names[s_mode]);
        }

        last_l1 = l1;
        last_l2 = l2;
        last_r1 = r1;
        last_r2 = r2;
        last_start = start;
        last_select = select;

        vTaskDelayUntil(&wake_time, delay_time);
    }
}

void mp3_player_init(void)
{
    xTaskCreate(mp3_player_task, "mp3_player_task", 1024 * 4, NULL, 5, NULL);
}

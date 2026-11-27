#include "web_control.h"
#include "web_shared.h"
#include "web_control_page.h"
#include "mp3_player.h"
#include "battery.h"

#include <Arduino.h>
#include <WebServer.h>
#include <stdio.h>

web_control_t web_control = {0};


static int8_t arg_axis(const char *name)
{
    long value = server.arg(name).toInt();
    if (value < -100) {
        value = -100;
    } else if (value > 100) {
        value = 100;
    }
    return (int8_t)value;
}

static uint8_t arg_button(const char *name)
{
    if (!server.hasArg(name)) {
        return 0;
    }
    return server.arg(name) == "0" ? 0 : 1;
}

static void handle_control_page()
{
    server.sendHeader("Cache-Control", "no-store");
    /* 页面已内联 CSS/JS，直接按 Arduino PROGMEM 方式发送 */
    server.send_P(200, "text/html; charset=utf-8", WEB_CONTROL_PAGE);
}

static void handle_control_state()
{
    web_control.lx = arg_axis("lx");
    web_control.ly = arg_axis("ly");
    web_control.rx = arg_axis("rx");
    web_control.ry = arg_axis("ry");

    web_control.up     = arg_button("up");
    web_control.down   = arg_button("down");
    web_control.left   = arg_button("left");
    web_control.right  = arg_button("right");

    web_control.triangle = arg_button("triangle");
    web_control.circle   = arg_button("circle");
    web_control.cross    = arg_button("cross");
    web_control.square   = arg_button("square");

    web_control.l1 = arg_button("l1");
    web_control.l2 = arg_button("l2");
    web_control.r1 = arg_button("r1");
    web_control.r2 = arg_button("r2");

    web_control.l3 = arg_button("l3");
    web_control.r3 = arg_button("r3");

    web_control.select = arg_button("select");
    web_control.start  = arg_button("start");
    web_control.ps     = arg_button("ps");

    server.send(200, "text/plain", "ok");

    /* 网页每次更新后立即输出变化，不依赖 loop() 轮询 */
    web_control_debug();
}

static void handle_control_status()
{
    char buf[64];
    int battery = battery_percent();
    int volume = (int)mp3_player_volume_percent();
    int playing = (int)mp3_player_is_playing();
    int track = (int)mp3_player_track();
    int mode = (int)mp3_player_mode();
    int files = (int)mp3_player_file_count();

    snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%d",
             battery, volume, playing, track, mode, files);

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain", buf);
}

void web_control_debug()
{
    static web_control_t last = {0};
    bool changed = false;

    #define DEBUG_AXIS(field) \
        if (web_control.field != last.field) { \
            if (!changed) { Serial.print("[CTRL]"); changed = true; } \
            Serial.printf(" %s=%d", #field, web_control.field); \
            last.field = web_control.field; \
        }

    #define DEBUG_BUTTON(field) \
        if (web_control.field != last.field) { \
            if (!changed) { Serial.print("[CTRL]"); changed = true; } \
            Serial.printf(" %s=%d", #field, web_control.field); \
            last.field = web_control.field; \
        }

    DEBUG_AXIS(lx);
    DEBUG_AXIS(ly);
    DEBUG_AXIS(rx);
    DEBUG_AXIS(ry);

    DEBUG_BUTTON(up);
    DEBUG_BUTTON(down);
    DEBUG_BUTTON(left);
    DEBUG_BUTTON(right);

    DEBUG_BUTTON(triangle);
    DEBUG_BUTTON(circle);
    DEBUG_BUTTON(cross);
    DEBUG_BUTTON(square);

    DEBUG_BUTTON(l1);
    DEBUG_BUTTON(l2);
    DEBUG_BUTTON(r1);
    DEBUG_BUTTON(r2);

    DEBUG_BUTTON(l3);
    DEBUG_BUTTON(r3);

    DEBUG_BUTTON(select);
    DEBUG_BUTTON(start);
    DEBUG_BUTTON(ps);

    if (changed) {
        Serial.println();
    }

    #undef DEBUG_AXIS
    #undef DEBUG_BUTTON
}

void web_control_register()
{
    server.on("/control", HTTP_GET, handle_control_page);
    server.on("/control/state", HTTP_GET, handle_control_state);
    server.on("/control/state", HTTP_POST, handle_control_state);
    server.on("/control/status", HTTP_GET, handle_control_status);
}

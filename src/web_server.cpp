#include "web_server.h"
#include "web_shared.h"
#include "web_wifi_config.h"
#include "web_control.h"

#include <Arduino.h>
#include <WebServer.h>

extern const uint8_t index_html_start[] asm("_binary_html_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_html_index_html_end");
extern const uint8_t style_css_start[]  asm("_binary_html_style_css_start");
extern const uint8_t style_css_end[]    asm("_binary_html_style_css_end");

/* 全局唯一的 WebServer */
WebServer server(80);

static void handle_home()
{
    String page = web_load_asset(index_html_start, index_html_end);
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8", page);
}

static void handle_css()
{
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/css; charset=utf-8",
                web_load_asset(style_css_start, style_css_end));
}

static void handle_not_found()
{
    server.send(404, "text/html; charset=utf-8",
                "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\">"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<title>页面不存在</title><link rel=\"stylesheet\" href=\"/style.css\"></head>"
                "<body><div class=\"container\"><h1>404 页面不存在</h1>"
                "<p><a href=\"/\">返回主页</a></p></div></body></html>");
}

static void web_server_task(void *args)
{
    for (;;) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void web_server_init()
{
    server.on("/", HTTP_GET, handle_home);
    server.on("/style.css", HTTP_GET, handle_css);

    web_wifi_config_register();
    web_control_register();

    server.onNotFound(handle_not_found);

    /* 必须先初始化 WiFi（lwIP/netif），再启动 WebServer */
    web_wifi_config_start();

    server.begin();
    Serial.println("[Web] 服务器已启动");

    xTaskCreate(web_server_task, "web_server_task", 1024 * 8, NULL, 10, NULL);
}

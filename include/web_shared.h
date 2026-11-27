#ifndef WEB_SHARED_H
#define WEB_SHARED_H

#include <Arduino.h>
#include <WebServer.h>

/* 全局唯一的 WebServer，由 web_server.cpp 创建 */
extern WebServer server;

String web_html_escape(const String &input);
String web_url_encode(const String &input);
String web_load_asset(const uint8_t *start, const uint8_t *end);

#endif

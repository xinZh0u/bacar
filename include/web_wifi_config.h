#ifndef WEB_WIFI_CONFIG_H
#define WEB_WIFI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 配网热点默认值；可在网页“配置热点”中修改，修改后保存在 NVS */
#define WEB_AP_SSID         "ESP32S3-Config"   // 默认热点名称
#define WEB_AP_PASSWORD     ""                 // 默认密码；留空为开放热点，至少 8 位则加密

/* 最多保存的 WiFi 数量 */
#define WEB_MAX_SAVED       8

/* 注册 /wifi 相关路由并读取已保存配置 */
void web_wifi_config_register();

/* 启动 WiFi 连接、配置热点及后台管理任务 */
void web_wifi_config_start();

#ifdef __cplusplus
}
#endif

#endif

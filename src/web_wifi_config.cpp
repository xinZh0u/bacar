#include "web_wifi_config.h"
#include "web_shared.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/* 由 platformio.ini 中的 board_build.embed_txtfiles 嵌入 */
extern const uint8_t wifi_html_start[] asm("_binary_html_wifi_html_start");
extern const uint8_t wifi_html_end[]   asm("_binary_html_wifi_html_end");
extern const uint8_t scan_html_start[] asm("_binary_html_scan_html_start");
extern const uint8_t scan_html_end[]   asm("_binary_html_scan_html_end");

/* 自动连接单个网络时等待的时间 */
#define WIFI_CONNECT_TIMEOUT_MS   10000
/* 一次自动选择的总时间上限 */
#define WIFI_AUTO_TOTAL_MS        60000
/* 自动重连检查间隔 */
#define WIFI_RETRY_INTERVAL_MS    30000

/* 扫描结果最多显示数量 */
#define WIFI_SCAN_MAX_SHOW        30

/* 优先级范围：数字越大越优先 */
#define WIFI_MAX_PRIORITY         9

Preferences prefs;
SemaphoreHandle_t wifi_mutex = nullptr;

typedef struct {
    String ssid;
    String password;
    uint8_t priority;
} saved_wifi_t;

static saved_wifi_t saved_wifi[WEB_MAX_SAVED];
static uint8_t saved_count = 0;
static String page_message;
static bool config_ap_started = false;
static String ap_ssid;
static String ap_password;
static bool ap_restart_pending = false;

/* ========================= NVS 保存 ========================= */

static void load_ap_config()
{
    prefs.begin("apcfg", true);
    ap_ssid = prefs.getString("ssid", WEB_AP_SSID);
    ap_password = prefs.getString("pass", WEB_AP_PASSWORD);
    prefs.end();

    if (ap_ssid.length() == 0 || ap_ssid.length() > 32) {
        ap_ssid = WEB_AP_SSID;
    }
    if (ap_password.length() > 63 ||
        (ap_password.length() > 0 && ap_password.length() < 8)) {
        ap_password = WEB_AP_PASSWORD;
    }
}

static void save_ap_config()
{
    prefs.begin("apcfg", false);
    prefs.putString("ssid", ap_ssid);
    prefs.putString("pass", ap_password);
    prefs.end();
}

static void load_saved_wifi()
{
    saved_count = 0;
    prefs.begin("wifi", true);
    saved_count = prefs.getUChar("count", 0);
    if (saved_count > WEB_MAX_SAVED) {
        saved_count = WEB_MAX_SAVED;
    }
    for (uint8_t i = 0; i < saved_count; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        String prio_key = "prio" + String(i);
        saved_wifi[i].ssid = prefs.getString(ssid_key.c_str(), "");
        saved_wifi[i].password = prefs.getString(pass_key.c_str(), "");
        saved_wifi[i].priority = prefs.getUChar(prio_key.c_str(), 0);
    }
    prefs.end();
}

static void save_saved_wifi()
{
    prefs.begin("wifi", false);
    prefs.putUChar("count", saved_count);
    for (uint8_t i = 0; i < WEB_MAX_SAVED; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        String prio_key = "prio" + String(i);
        if (i < saved_count) {
            prefs.putString(ssid_key.c_str(), saved_wifi[i].ssid);
            prefs.putString(pass_key.c_str(), saved_wifi[i].password);
            prefs.putUChar(prio_key.c_str(), saved_wifi[i].priority);
        } else {
            prefs.remove(ssid_key.c_str());
            prefs.remove(pass_key.c_str());
            prefs.remove(prio_key.c_str());
        }
    }
    prefs.end();
}

/* 按优先级从高到低生成下标顺序 */
static int priority_order(int *order)
{
    for (uint8_t i = 0; i < saved_count; i++) {
        order[i] = i;
    }
    /* 插入排序，同优先级时保持原有顺序 */
    for (int i = 1; i < saved_count; i++) {
        int key = order[i];
        int j = i - 1;
        while (j >= 0 &&
               saved_wifi[order[j]].priority < saved_wifi[key].priority) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }
    return saved_count;
}

static int find_saved_wifi(const String &ssid)
{
    for (uint8_t i = 0; i < saved_count; i++) {
        if (saved_wifi[i].ssid == ssid) {
            return i;
        }
    }
    return -1;
}

static bool delete_saved_wifi(const String &ssid)
{
    int index = find_saved_wifi(ssid);
    if (index < 0) {
        return false;
    }
    for (uint8_t i = index; i + 1 < saved_count; i++) {
        saved_wifi[i] = saved_wifi[i + 1];
    }
    saved_count--;
    save_saved_wifi();
    return true;
}

/* ========================= WiFi 连接 ========================= */

static bool wifi_connect(const String &ssid, const String &password, uint32_t timeout_ms)
{
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        return true;
    }

    WiFi.disconnect(false, false);
    delay(100);

    if (password.length() == 0) {
        WiFi.begin(ssid.c_str());
    } else {
        WiFi.begin(ssid.c_str(), password.c_str());
    }

    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
            Serial.print("[WiFi] 已连接: ");
            Serial.print(ssid);
            Serial.print("  IP: ");
            Serial.println(WiFi.localIP());
            return true;
        }
        /* 密码错误或找不到 AP 时不必等到超时 */
        if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
            break;
        }
        delay(200);
    }

    WiFi.disconnect(false, false);
    return false;
}

/*
 * 自动选择 WiFi：
 * 1. 扫描附近网络；
 * 2. 找出已保存且当前存在的网络，先按优先级从高到低、同优先级再按
 *    信号强度从强到弱尝试；
 * 3. 最后按优先级尝试已保存但未扫描到的网络（例如隐藏网络）。
 */
static bool auto_connect_saved()
{
    if (saved_count == 0) {
        return false;
    }

    int found_index[WEB_MAX_SAVED];
    int found_rssi[WEB_MAX_SAVED];
    bool tried[WEB_MAX_SAVED];
    int found_num = 0;

    for (uint8_t i = 0; i < saved_count; i++) {
        tried[i] = false;
    }

    Serial.println("[WiFi] 扫描附近网络...");
    int n = WiFi.scanNetworks(false, true);

    for (uint8_t i = 0; i < saved_count; i++) {
        int best_rssi = -1000;
        for (int j = 0; j < n; j++) {
            if (WiFi.SSID(j) == saved_wifi[i].ssid && WiFi.RSSI(j) > best_rssi) {
                best_rssi = WiFi.RSSI(j);
            }
        }
        if (best_rssi > -1000) {
            found_index[found_num] = i;
            found_rssi[found_num] = best_rssi;
            found_num++;
        }
    }

    WiFi.scanDelete();

    /* 先按优先级从高到低排序，同优先级再按信号从强到弱排序 */
    for (int i = 0; i < found_num - 1; i++) {
        for (int j = i + 1; j < found_num; j++) {
            uint8_t prio_i = saved_wifi[found_index[i]].priority;
            uint8_t prio_j = saved_wifi[found_index[j]].priority;
            if (prio_j > prio_i ||
                (prio_j == prio_i && found_rssi[j] > found_rssi[i])) {
                int tmp_i = found_index[i];
                found_index[i] = found_index[j];
                found_index[j] = tmp_i;
                int tmp_r = found_rssi[i];
                found_rssi[i] = found_rssi[j];
                found_rssi[j] = tmp_r;
            }
        }
    }

    uint32_t start = millis();

    /* 优先连接扫描到的已保存网络 */
    for (int k = 0; k < found_num; k++) {
        if (millis() - start > WIFI_AUTO_TOTAL_MS) {
            return false;
        }
        int i = found_index[k];
        tried[i] = true;
        Serial.print("[WiFi] 尝试连接: ");
        Serial.println(saved_wifi[i].ssid);
        if (wifi_connect(saved_wifi[i].ssid, saved_wifi[i].password, WIFI_CONNECT_TIMEOUT_MS)) {
            return true;
        }
    }

    /* 再按优先级尝试未扫描到的已保存网络（隐藏网络） */
    int order[WEB_MAX_SAVED];
    int order_num = priority_order(order);
    for (int k = 0; k < order_num; k++) {
        int i = order[k];
        if (tried[i]) {
            continue;
        }
        if (millis() - start > WIFI_AUTO_TOTAL_MS) {
            return false;
        }
        Serial.print("[WiFi] 尝试连接(隐藏): ");
        Serial.println(saved_wifi[i].ssid);
        if (wifi_connect(saved_wifi[i].ssid, saved_wifi[i].password, WIFI_CONNECT_TIMEOUT_MS)) {
            return true;
        }
    }

    return false;
}

static bool apply_soft_ap()
{
    if (ap_password.length() >= 8) {
        return WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    }
    return WiFi.softAP(ap_ssid.c_str());
}

static void start_config_ap()
{
    if (config_ap_started) {
        return;
    }

    WiFi.mode(WIFI_AP_STA);

    bool ok = apply_soft_ap();

    /* 如果芯片沿用了默认热点名 ESP_XXXXXX，重启一次 AP 让新名称生效 */
    if (ok && WiFi.softAPSSID() != ap_ssid) {
        Serial.print("[WiFi] 热点名未生效，实际为: ");
        Serial.println(WiFi.softAPSSID());
        WiFi.softAPdisconnect(true);
        delay(200);
        WiFi.mode(WIFI_AP_STA);
        ok = apply_soft_ap();
    }

    if (ok) {
        config_ap_started = true;
        Serial.print("[WiFi] 配置热点已开启: ");
        Serial.print(WiFi.softAPSSID());
        Serial.print("  IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("[WiFi] 配置热点开启失败");
    }
}

static void restart_config_ap()
{
    config_ap_started = false;
    WiFi.softAPdisconnect(false);
    start_config_ap();
}

/* ========================= 页面内容 ========================= */

static String wifi_status_html()
{
    String status;
    if (WiFi.status() == WL_CONNECTED) {
        status = "已连接到 <strong>";
        status += web_html_escape(WiFi.SSID());
        status += "</strong>，IP 地址：";
        status += WiFi.localIP().toString();
    } else {
        status = "未连接";
    }

    if (config_ap_started) {
        status += "。配置热点：<strong>";
        status += web_html_escape(ap_ssid);
        status += "</strong>（http://";
        status += WiFi.softAPIP().toString();
        status += "）";
    }
    return status;
}

static String network_list_html()
{
    if (saved_count == 0) {
        return "        <li class=\"empty\">暂无已保存的网络</li>";
    }

    int order[WEB_MAX_SAVED];
    int order_num = priority_order(order);

    String html;
    for (int k = 0; k < order_num; k++) {
        int i = order[k];
        String encoded_ssid = web_url_encode(saved_wifi[i].ssid);

        html += "        <li class=\"network-item\">\n";
        html += "          <div class=\"network-info\">\n";
        html += "            <span class=\"network-name\">";
        html += web_html_escape(saved_wifi[i].ssid);
        html += "</span>\n";
        html += "            <span class=\"network-meta\">优先级 ";
        html += String(saved_wifi[i].priority);
        if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == saved_wifi[i].ssid) {
            html += " · 当前连接";
        }
        html += "</span>\n";
        html += "          </div>\n";
        html += "          <div class=\"network-actions\">\n";

        if (saved_wifi[i].priority < WIFI_MAX_PRIORITY) {
            html += "            <a class=\"icon-button\" href=\"/wifi/priority?ssid=";
            html += encoded_ssid;
            html += "&amp;delta=1\" title=\"提高优先级\" aria-label=\"提高优先级\">↑</a>\n";
        } else {
            html += "            <span class=\"icon-button disabled\" aria-hidden=\"true\">↑</span>\n";
        }

        if (saved_wifi[i].priority > 0) {
            html += "            <a class=\"icon-button\" href=\"/wifi/priority?ssid=";
            html += encoded_ssid;
            html += "&amp;delta=-1\" title=\"降低优先级\" aria-label=\"降低优先级\">↓</a>\n";
        } else {
            html += "            <span class=\"icon-button disabled\" aria-hidden=\"true\">↓</span>\n";
        }

        html += "            <a class=\"delete\" href=\"/wifi/delete?ssid=";
        html += encoded_ssid;
        html += "\">删除</a>\n";
        html += "          </div>\n";
        html += "        </li>\n";
    }
    return html;
}

static void redirect_wifi_page()
{
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
}

/* ========================= 请求处理 ========================= */

static void handle_wifi_page()
{
    String page = web_load_asset(wifi_html_start, wifi_html_end);
    String message = page_message;
    page_message = "";

    page.replace("<!--WIFI_STATUS-->", wifi_status_html());
    page.replace("<!--MESSAGE-->", message);
    page.replace("<!--NETWORK_LIST-->", network_list_html());
    page.replace("<!--AP_SSID-->", web_html_escape(ap_ssid));

    server.send(200, "text/html; charset=utf-8", page);
}

static void handle_save()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0 || ssid.length() > 32) {
        page_message = "保存失败：SSID 不能为空，且长度不能超过 32 字节。";
        redirect_wifi_page();
        return;
    }

    String priority_arg = server.arg("priority");
    bool has_priority = priority_arg.length() > 0;
    uint8_t priority = 0;
    if (has_priority) {
        long value = priority_arg.toInt();
        if (value < 0) {
            value = 0;
        } else if (value > WIFI_MAX_PRIORITY) {
            value = WIFI_MAX_PRIORITY;
        }
        priority = (uint8_t)value;
    }

    int index = find_saved_wifi(ssid);
    bool is_new = (index < 0);
    if (is_new) {
        if (saved_count >= WEB_MAX_SAVED) {
            page_message = "保存失败：最多保存 " + String(WEB_MAX_SAVED) +
                           " 个网络，请先删除不用的网络。";
            redirect_wifi_page();
            return;
        }
        index = saved_count;
        saved_count++;
    }

    saved_wifi[index].ssid = ssid;
    saved_wifi[index].password = password;
    if (has_priority) {
        saved_wifi[index].priority = priority;
    } else if (is_new) {
        /* 新添加的网络默认最高优先级 */
        saved_wifi[index].priority = WIFI_MAX_PRIORITY;
    }
    save_saved_wifi();

    bool connected = false;
    if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(30000)) == pdTRUE) {
        connected = wifi_connect(ssid, password, WIFI_CONNECT_TIMEOUT_MS);
        xSemaphoreGive(wifi_mutex);
    }

    if (connected) {
        page_message = "已保存并连接到 <strong>" + web_html_escape(ssid) + "</strong>。";
    } else {
        page_message = "已保存 <strong>" + web_html_escape(ssid) +
                       "</strong>，但连接失败，请检查密码或信号。";
    }

    redirect_wifi_page();
}

static void handle_priority()
{
    String ssid = server.arg("ssid");
    int index = find_saved_wifi(ssid);
    if (index < 0) {
        page_message = "调整失败：未找到该网络。";
        redirect_wifi_page();
        return;
    }

    int delta = server.arg("delta").toInt();
    if (delta == 0) {
        delta = 1;
    }

    int value = (int)saved_wifi[index].priority + (delta > 0 ? 1 : -1);
    if (value < 0) {
        value = 0;
    } else if (value > WIFI_MAX_PRIORITY) {
        value = WIFI_MAX_PRIORITY;
    }

    if (value == saved_wifi[index].priority) {
        page_message = "网络 <strong>" + web_html_escape(ssid) + "</strong> 的优先级已经是 " +
                       String(value) + "，无法继续调整。";
    } else {
        saved_wifi[index].priority = (uint8_t)value;
        save_saved_wifi();
        page_message = "已将 <strong>" + web_html_escape(ssid) + "</strong> 的优先级调整为 " +
                       String(value) + "（数字越大越优先，下次自动连接时生效）。";
    }

    redirect_wifi_page();
}

static void ap_restart_task(void *args)
{
    /* 等浏览器把响应收完，再重启热点 */
    vTaskDelay(pdMS_TO_TICKS(1500));

    if (xSemaphoreTake(wifi_mutex, portMAX_DELAY) == pdTRUE) {
        ap_restart_pending = false;
        Serial.println("[WiFi] 应用新的热点配置...");
        restart_config_ap();
        xSemaphoreGive(wifi_mutex);
    }

    vTaskDelete(NULL);
}

static void handle_ap()
{
    String ssid = server.arg("ap_ssid");
    String password = server.arg("ap_password");
    bool make_open = server.hasArg("open");

    if (ssid.length() == 0 || ssid.length() > 32) {
        page_message = "热点保存失败：名称不能为空，且长度不能超过 32 字节。";
        redirect_wifi_page();
        return;
    }

    String new_password = ap_password;
    if (make_open) {
        new_password = "";
    } else if (password.length() > 0) {
        if (password.length() < 8 || password.length() > 63) {
            page_message = "热点保存失败：密码需为 8-63 位；留空表示保持当前密码。";
            redirect_wifi_page();
            return;
        }
        new_password = password;
    }

    bool changed = (ssid != ap_ssid) || (new_password != ap_password);
    ap_ssid = ssid;
    ap_password = new_password;
    save_ap_config();

    if (changed) {
        page_message = "热点设置已保存，正在重启热点，请用新的名称和密码重新连接。";
        if (!ap_restart_pending) {
            ap_restart_pending = true;
            xTaskCreate(ap_restart_task, "ap_restart_task", 1024 * 3, NULL, 3, NULL);
        }
    } else {
        page_message = "热点设置未变化。";
    }

    redirect_wifi_page();
}

static void handle_delete()
{
    String ssid = server.arg("ssid");

    if (delete_saved_wifi(ssid)) {
        page_message = "已删除网络 <strong>" + web_html_escape(ssid) + "</strong>。";
        if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
            page_message += " 当前连接会保持到重启。";
        }
    } else {
        page_message = "删除失败：未找到该网络。";
    }

    redirect_wifi_page();
}

static void handle_scan()
{
    if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(60000)) != pdTRUE) {
        server.send(503, "text/html; charset=utf-8",
                    "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\">"
                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                    "<title>设备忙</title><link rel=\"stylesheet\" href=\"/style.css\"></head>"
                    "<body><div class=\"container\"><h1>设备正忙</h1>"
                    "<p>WiFi 正在连接中，请稍后<a href=\"/wifi/scan\">重试</a>。</p>"
                    "</div></body></html>");
        return;
    }

    String ssids[WIFI_SCAN_MAX_SHOW];
    int rssis[WIFI_SCAN_MAX_SHOW];
    bool encrypted[WIFI_SCAN_MAX_SHOW];
    int count = 0;
    bool truncated = false;

    int n = WiFi.scanNetworks(false, true);

    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) {
            continue;
        }

        int found = -1;
        for (int k = 0; k < count; k++) {
            if (ssids[k] == ssid) {
                found = k;
                break;
            }
        }

        int rssi = WiFi.RSSI(i);
        bool enc = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);

        if (found >= 0) {
            if (rssi > rssis[found]) {
                rssis[found] = rssi;
                encrypted[found] = enc;
            }
        } else if (count < WIFI_SCAN_MAX_SHOW) {
            ssids[count] = ssid;
            rssis[count] = rssi;
            encrypted[count] = enc;
            count++;
        } else {
            truncated = true;
        }
    }

    WiFi.scanDelete();

    /* 按信号强度从强到弱排序 */
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (rssis[j] > rssis[i]) {
                String tmp_s = ssids[i];
                ssids[i] = ssids[j];
                ssids[j] = tmp_s;
                int tmp_r = rssis[i];
                rssis[i] = rssis[j];
                rssis[j] = tmp_r;
                bool tmp_e = encrypted[i];
                encrypted[i] = encrypted[j];
                encrypted[j] = tmp_e;
            }
        }
    }

    String summary;
    if (count == 0) {
        summary = "未发现可用的 WiFi 网络，请靠近路由器后重试。";
    } else {
        summary = "共发现 " + String(count) +
                  " 个网络，点击“连接”即可保存（新网络默认最高优先级）并连接。";
        if (truncated) {
            summary += "（仅显示前 " + String(count) + " 个）";
        }
    }

    String list;
    for (int i = 0; i < count; i++) {
        String id = "pwd" + String(i);

        list += "        <li class=\"network-item scan\">\n";
        list += "          <div class=\"network-info\">\n";
        list += "            <span class=\"network-name\">";
        list += web_html_escape(ssids[i]);
        list += "</span>\n";
        list += "            <span class=\"network-meta\">信号 ";
        list += String(rssis[i]);
        list += " dBm · ";
        list += encrypted[i] ? "加密" : "开放";
        list += "</span>\n";
        list += "          </div>\n";

        list += "          <form class=\"scan-form\" action=\"/wifi/save\" method=\"post\">\n";
        list += "            <input type=\"hidden\" name=\"ssid\" value=\"";
        list += web_html_escape(ssids[i]);
        list += "\">\n";
        list += "            <label class=\"sr-only\" for=\"";
        list += id;
        list += "\">密码</label>\n";
        list += "            <input type=\"password\" id=\"";
        list += id;
        list += "\" name=\"password\" maxlength=\"64\" placeholder=\"密码\">\n";
        list += "            <button type=\"submit\">连接</button>\n";
        list += "          </form>\n";
        list += "        </li>\n";
    }
    if (count == 0) {
        list = "        <li class=\"empty\">没有扫描结果</li>\n";
    }

    xSemaphoreGive(wifi_mutex);

    String page = web_load_asset(scan_html_start, scan_html_end);
    page.replace("<!--SCAN_SUMMARY-->", summary);
    page.replace("<!--SCAN_LIST-->", list);

    server.send(200, "text/html; charset=utf-8", page);
}

/* ========================= 任务 ========================= */

static void wifi_manager_task(void *args)
{
    for (;;) {
        /* 先确保热点开启，再处理 STA 重连 */
        if (!config_ap_started && !ap_restart_pending) {
            Serial.println("[WiFi] 热点未开启，正在重试...");
            start_config_ap();
        }

        if (WiFi.status() != WL_CONNECTED && saved_count > 0) {
            if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("[WiFi] 未连接，尝试自动选择网络...");
                    auto_connect_saved();
                }
                xSemaphoreGive(wifi_mutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_INTERVAL_MS));
    }
}

/* ========================= 注册与启动 ========================= */

void web_wifi_config_register()
{
    if (!wifi_mutex) {
        wifi_mutex = xSemaphoreCreateMutex();
    }

    load_saved_wifi();
    load_ap_config();

    server.on("/wifi", HTTP_GET, handle_wifi_page);
    server.on("/wifi/save", HTTP_POST, handle_save);
    server.on("/wifi/save", HTTP_GET, handle_save);
    server.on("/wifi/priority", HTTP_GET, handle_priority);
    server.on("/wifi/ap", HTTP_POST, handle_ap);
    server.on("/wifi/delete", HTTP_GET, handle_delete);
    server.on("/wifi/scan", HTTP_GET, handle_scan);
}

void web_wifi_config_start()
{
    Serial.println("[WiFi] 初始化...");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    /* 配置热点始终开启：AP 与 STA 同时工作 */
    start_config_ap();

    /*
     * 自动连接放到后台任务里执行，避免阻塞 setup()，
     * WebServer 可以立即启动并响应配置页面。
     */
    xTaskCreate(wifi_manager_task, "wifi_manager_task", 1024 * 4, NULL, 3, NULL);
}

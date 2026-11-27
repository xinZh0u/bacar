// 自动生成，请勿直接修改。
// 源文件：html/control.html + control.css + control.js + style.css
// 重新生成：python3 tools/gen_control_page.py
#pragma once

#include <Arduino.h>

static const char WEB_CONTROL_PAGE[] PROGMEM = R"PAGE_RAW(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="Cache-Control" content="no-store">
<title>手柄控制 - ESP32-S3</title>
<style>
:root {
  color-scheme: light;
}

* {
  box-sizing: border-box;
}

html {
  -webkit-text-size-adjust: 100%;
}

body {
  margin: 0;
  background: #fff;
  color: #111;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto,
               "Helvetica Neue", Arial, "Noto Sans SC", "Microsoft YaHei", sans-serif;
  font-size: 15px;
  line-height: 1.6;
}

.container {
  max-width: 640px;
  margin: 0 auto;
  padding: 40px 20px 64px;
}

header {
  margin-bottom: 32px;
}

h1 {
  margin: 0 0 8px;
  font-size: 24px;
  font-weight: 600;
  letter-spacing: -0.02em;
}

h2 {
  margin: 0 0 14px;
  font-size: 16px;
  font-weight: 600;
}

p {
  margin: 0 0 14px;
  color: #444;
}

strong {
  color: #111;
  font-weight: 600;
}

section {
  margin: 32px 0;
}

hr {
  margin: 32px 0;
  border: 0;
  border-top: 1px solid #ededed;
}

a {
  color: #111;
  text-decoration: underline;
  text-decoration-thickness: 1px;
  text-underline-offset: 3px;
}

a:hover {
  color: #666;
}

form {
  margin: 0;
}

.field {
  margin-bottom: 16px;
}

label {
  display: block;
  margin-bottom: 6px;
  font-size: 13px;
  font-weight: 500;
}

input[type="text"],
input[type="password"],
input[type="number"] {
  width: 100%;
  padding: 10px 12px;
  color: #111;
  background: #fff;
  border: 1px solid #ddd;
  border-radius: 8px;
  outline: none;
  font: inherit;
  transition: border-color 0.15s ease;
}

input[type="text"]:focus,
input[type="password"]:focus,
input[type="number"]:focus {
  border-color: #111;
}

.checkbox {
  display: flex;
  align-items: center;
  gap: 8px;
  margin: 0;
  font-weight: 400;
}

.checkbox input[type="checkbox"] {
  width: 16px;
  height: 16px;
  margin: 0;
  accent-color: #111;
}

button {
  padding: 10px 18px;
  color: #fff;
  background: #111;
  border: 1px solid #111;
  border-radius: 8px;
  font: inherit;
  font-weight: 500;
  cursor: pointer;
  transition: background 0.15s ease, border-color 0.15s ease;
}

button:hover {
  background: #333;
  border-color: #333;
}

button:focus-visible,
a:focus-visible,
input:focus-visible {
  outline: 2px solid #111;
  outline-offset: 2px;
}

.hint {
  margin: 6px 0 0;
  color: #888;
  font-size: 12px;
}

.status {
  margin: 0;
  color: #666;
  font-size: 14px;
}

.notice {
  padding: 10px 12px;
  background: #fafafa;
  border: 1px solid #eee;
  border-radius: 8px;
  color: #444;
  font-size: 13px;
}

.notice:empty {
  display: none;
}

.menu-list {
  margin: 0;
  padding: 0;
  list-style: none;
  border-top: 1px solid #eee;
}

.menu-item {
  border-bottom: 1px solid #eee;
}

.menu-link {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 18px 0;
  color: #111;
  text-decoration: none;
}

.menu-link:hover {
  color: #666;
}

.menu-text {
  flex: 1;
}

.menu-title {
  display: block;
  font-weight: 500;
}

.menu-desc {
  display: block;
  margin-top: 2px;
  color: #888;
  font-size: 12px;
}

.menu-arrow {
  color: #aaa;
  font-size: 18px;
}

.network-list {
  margin: 0;
  padding: 0;
  list-style: none;
  border-top: 1px solid #eee;
}

.network-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  padding: 14px 0;
  border-bottom: 1px solid #eee;
}

.network-name {
  display: block;
  font-weight: 500;
  word-break: break-all;
}

.network-meta {
  display: block;
  margin-top: 2px;
  color: #888;
  font-size: 12px;
}

.network-actions {
  display: flex;
  flex-shrink: 0;
  align-items: center;
  gap: 8px;
}

.icon-button {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 30px;
  height: 30px;
  border: 1px solid #ddd;
  border-radius: 6px;
  color: #111;
  font-size: 15px;
  line-height: 1;
  text-decoration: none;
}

.icon-button:hover {
  border-color: #111;
  color: #111;
}

.icon-button.disabled {
  color: #ddd;
  border-color: #f0f0f0;
  cursor: default;
}

.delete {
  margin-left: 4px;
  color: #888;
  font-size: 13px;
  text-decoration: none;
}

.delete:hover {
  color: #111;
}

.empty {
  padding: 14px 0;
  color: #888;
  font-size: 13px;
}

.actions-nav {
  display: flex;
  flex-wrap: wrap;
  gap: 20px;
  margin: 0;
  padding: 0;
  list-style: none;
}

.network-item.scan {
  display: block;
}

.scan-form {
  display: flex;
  gap: 8px;
  margin-top: 10px;
}

.scan-form input {
  flex: 1;
  min-width: 0;
}

.scan-form button {
  flex-shrink: 0;
  white-space: nowrap;
}

footer {
  margin-top: 48px;
  color: #aaa;
  font-size: 12px;
}

footer p {
  margin: 0;
  color: #aaa;
}

.sr-only {
  position: absolute;
  width: 1px;
  height: 1px;
  padding: 0;
  margin: -1px;
  overflow: hidden;
  clip: rect(0, 0, 0, 0);
  white-space: nowrap;
  border: 0;
}

@media (max-width: 480px) {
  .container {
    padding: 28px 16px 48px;
  }

  h1 {
    font-size: 21px;
  }

  .network-actions {
    gap: 6px;
  }
}

</style>
<style>
/* ===== PS3 布局虚拟手柄 · 自适应窗口 · 极简白色 ===== */

body {
  overflow: hidden;
}

.control-page {
  --page-gap: clamp(8px, 1.8vmin, 20px);
  width: 100%;
  max-width: none;
  height: 100vh;
  height: 100dvh;
  margin: 0;
  padding: var(--page-gap);
  display: flex;
  justify-content: center;
}

.control-page main {
  flex: 1;
  min-height: 0;
  display: flex;
}

.ctrl-pad {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  width: 100%;
  max-width: none;
  min-height: 0;
  margin: 0;
  padding: clamp(14px, 2.6vmin, 30px) clamp(18px, 4vw, 56px);
  background: #fff;
  border: 1px solid #e5e5e5;
  border-radius: clamp(14px, 2.6vmin, 28px);
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.03);
  gap: clamp(10px, 2vmin, 24px);
  overflow: hidden;
  user-select: none;
  -webkit-user-select: none;
  touch-action: none;
}

.ctrl-shoulders,
.ctrl-main,
.ctrl-sticks {
  width: 100%;
}

.ctrl-shoulders {
  display: flex;
  justify-content: space-between;
  flex-shrink: 0;
}

.ctrl-trigger-group {
  display: flex;
  gap: clamp(4px, 1vmin, 10px);
}

.ctrl-pad button {
  font-family: inherit;
  color: #111;
  background: #fff;
  border: 1px solid #ddd;
  cursor: pointer;
  touch-action: none;
  -webkit-tap-highlight-color: transparent;
}

.ctrl-pad button:hover {
  color: #111;
  background: #fafafa;
  border-color: #cfcfcf;
}

.ctrl-pad button.active:hover,
.ctrl-pad button:active:hover {
  color: #fff;
  background: #111;
  border-color: #111;
}

.ctrl-pad button:focus-visible {
  outline: 2px solid #111;
  outline-offset: 2px;
}

.ctrl-trigger {
  min-width: clamp(48px, 13vmin, 143px);
  padding: clamp(6px, 1.8vmin, 18px) clamp(10px, 2.6vmin, 31px);
  border-radius: 9px;
  font-size: clamp(10px, 2.3vmin, 22px);
  font-weight: 500;
}

/* ---- 十字键 / 中间键 / 功能键 ---- */

.ctrl-main {
  flex: 1;
  min-height: 0;
  display: grid;
  grid-template-columns: minmax(0, 1fr) auto minmax(0, 1fr);
  align-items: center;
  gap: clamp(8px, 2vw, 32px);
}

.ctrl-dpad {
  display: grid;
  grid-template-columns: repeat(3, clamp(38px, 11.7vmin, 104px));
  grid-template-rows: repeat(3, clamp(38px, 11.7vmin, 104px));
  gap: clamp(3px, 1vmin, 12px);
  justify-self: start;
}

.ctrl-dpad-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0;
  border-radius: 8px;
  background: #fafafa;
  font-size: clamp(11px, 2.3vmin, 22px);
  line-height: 1;
}

.ctrl-up    { grid-area: 1 / 2; }
.ctrl-left  { grid-area: 2 / 1; }
.ctrl-right { grid-area: 2 / 3; }
.ctrl-down  { grid-area: 3 / 2; }

.ctrl-dpad-center {
  grid-area: 2 / 2;
  display: block;
  border-radius: 8px;
  background: #f4f4f4;
}

.ctrl-center {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: clamp(6px, 1.8vmin, 26px);
}

.ctrl-info {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.ctrl-sysinfo {
  color: #555;
  font-size: clamp(9px, 1.4vmin, 16px);
  line-height: 1.2;
  letter-spacing: 0.02em;
  white-space: nowrap;
  text-align: center;
}

.ctrl-center-row {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: clamp(6px, 1.6vmin, 21px);
}

.ctrl-home {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: clamp(4px, 1.3vmin, 13px) clamp(8px, 2.1vmin, 21px);
  color: #666;
  background: #fff;
  border: 1px solid #ddd;
  border-radius: 7px;
  font-size: clamp(9px, 1.6vmin, 18px);
  white-space: nowrap;
  text-decoration: none;
}

.ctrl-home:hover {
  color: #111;
  border-color: #111;
}

.ctrl-home:focus-visible {
  outline: 2px solid #111;
  outline-offset: 2px;
}

.ctrl-small {
  padding: clamp(4px, 1.3vmin, 13px) clamp(7px, 1.8vmin, 18px);
  border-radius: 8px;
  font-size: clamp(8px, 1.4vmin, 17px);
  font-weight: 500;
  letter-spacing: 0.04em;
}

.ctrl-ps {
  width: clamp(42px, 6vmin, 75px);
  height: clamp(42px, 6vmin, 75px);
  padding: 0;
  border-radius: 50%;
  font-size: clamp(12px, 1.8vmin, 21px);
  font-weight: 600;
}

/* ---- 右侧功能键 ---- */

.ctrl-face {
  display: grid;
  grid-template-columns: repeat(3, clamp(44px, 14.3vmin, 120px));
  grid-template-rows: repeat(3, clamp(44px, 14.3vmin, 120px));
  gap: clamp(3px, 1.3vmin, 13px);
  justify-self: end;
}

.ctrl-face-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  width: clamp(44px, 14.3vmin, 120px);
  height: clamp(44px, 14.3vmin, 120px);
  padding: 0;
  border-radius: 50%;
  background: #fff;
  font-size: clamp(14px, 3.9vmin, 42px);
  line-height: 1;
}

.ctrl-triangle { grid-area: 1 / 2; }
.ctrl-square   { grid-area: 2 / 1; }
.ctrl-circle   { grid-area: 2 / 3; }
.ctrl-cross    { grid-area: 3 / 2; }

/* ---- 摇杆 ---- */

.ctrl-sticks {
  display: grid;
  grid-template-columns: 1fr 1fr;
  align-items: center;
  gap: 0;
  flex-shrink: 0;
}

.ctrl-stick-unit {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-self: center;
}

.ctrl-stick {
  position: relative;
  width: clamp(108px, 36.4vmin, 494px);
  height: clamp(108px, 36.4vmin, 494px);
  border: 1px solid #ddd;
  border-radius: 50%;
  background: #fafafa;
  touch-action: none;
  cursor: grab;
}

.ctrl-stick:active {
  cursor: grabbing;
}

.ctrl-knob {
  position: absolute;
  left: 50%;
  top: 50%;
  width: clamp(44px, 15.6vmin, 221px);
  height: clamp(44px, 15.6vmin, 221px);
  border: 1px solid #cfcfcf;
  border-radius: 50%;
  background: #fff;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.08);
  transform: translate(-50%, -50%);
  cursor: grab;
  touch-action: none;
}

.ctrl-stick:active .ctrl-knob {
  cursor: grabbing;
}

/* ---- 按下状态 ---- */

.ctrl-pad button.active,
.ctrl-pad button:active {
  color: #fff;
  background: #111;
  border-color: #111;
}

/* ---- 窄屏：只调整中间按键排列，不突然改控件尺寸 ---- */

@media (max-width: 560px) {
  .ctrl-main {
    gap: 6px;
  }

  .ctrl-center-row {
    flex-direction: column;
    gap: 5px;
  }

  .ctrl-trigger-group {
    gap: 3px;
  }

  .ctrl-trigger {
    min-width: 44px;
    padding: 5px 7px;
    font-size: 10px;
  }

  .ctrl-dpad {
    grid-template-columns: repeat(3, 40px);
    grid-template-rows: repeat(3, 40px);
  }

  .ctrl-face {
    grid-template-columns: repeat(3, 46px);
    grid-template-rows: repeat(3, 46px);
  }

  .ctrl-face-btn {
    width: 46px;
    height: 46px;
    font-size: 18px;
  }

  .ctrl-ps {
    width: 36px;
    height: 36px;
    font-size: 11px;
  }

  .ctrl-small {
    padding: 5px 7px;
    font-size: 9px;
  }

  .ctrl-home {
    padding: 5px 8px;
    font-size: 10px;
  }
}

</style>
</head>
<body>

<noscript>
  <p style="padding:16px;border:1px solid #ddd;">当前浏览器未启用 JavaScript，手柄无法使用。</p>
</noscript>

<div class="container control-page">

  <main>

    <div class="ctrl-pad" aria-label="虚拟手柄">

      <!-- 肩键 -->
      <div class="ctrl-shoulders">
        <div class="ctrl-trigger-group">
          <button type="button" class="ctrl-trigger" data-btn="l2" aria-label="L2">L2</button>
          <button type="button" class="ctrl-trigger" data-btn="l1" aria-label="L1">L1</button>
          <button type="button" class="ctrl-trigger" data-btn="l3" aria-label="L3">L3</button>
        </div>
        <div class="ctrl-trigger-group">
          <button type="button" class="ctrl-trigger" data-btn="r3" aria-label="R3">R3</button>
          <button type="button" class="ctrl-trigger" data-btn="r1" aria-label="R1">R1</button>
          <button type="button" class="ctrl-trigger" data-btn="r2" aria-label="R2">R2</button>
        </div>
      </div>

      <!-- 十字键 / 中间键 / 功能键 -->
      <div class="ctrl-main">

        <div class="ctrl-dpad" aria-label="方向键">
          <button type="button" class="ctrl-dpad-btn ctrl-up" data-btn="up" aria-label="上">▲</button>
          <button type="button" class="ctrl-dpad-btn ctrl-left" data-btn="left" aria-label="左">◀</button>
          <span class="ctrl-dpad-center" aria-hidden="true"></span>
          <button type="button" class="ctrl-dpad-btn ctrl-right" data-btn="right" aria-label="右">▶</button>
          <button type="button" class="ctrl-dpad-btn ctrl-down" data-btn="down" aria-label="下">▼</button>
        </div>

        <div class="ctrl-center">
          <div class="ctrl-info">
            <div class="ctrl-sysinfo" id="ctrl-sysinfo">电量--%  音量--%</div>
            <div class="ctrl-sysinfo" id="ctrl-playinfo">音频数量:-- 已暂停：第1首  单曲循环</div>
          </div>
          <div class="ctrl-center-row">
            <button type="button" class="ctrl-small" data-btn="select" aria-label="Select">SELECT</button>
            <button type="button" class="ctrl-ps" data-btn="ps" aria-label="PS">PS</button>
            <button type="button" class="ctrl-small" data-btn="start" aria-label="Start">START</button>
          </div>
          <a class="ctrl-home" href="/">返回主菜单</a>
        </div>

        <div class="ctrl-face" aria-label="功能键">
          <button type="button" class="ctrl-face-btn ctrl-triangle" data-btn="triangle" aria-label="三角">△</button>
          <button type="button" class="ctrl-face-btn ctrl-square" data-btn="square" aria-label="方块">□</button>
          <button type="button" class="ctrl-face-btn ctrl-circle" data-btn="circle" aria-label="圆圈">○</button>
          <button type="button" class="ctrl-face-btn ctrl-cross" data-btn="cross" aria-label="叉">✕</button>
        </div>

      </div>

      <!-- 双摇杆 -->
      <div class="ctrl-sticks">

        <div class="ctrl-stick-unit">
          <div class="ctrl-stick" id="stick-left" role="slider" aria-label="左摇杆">
            <div class="ctrl-knob"></div>
          </div>
        </div>

        <div class="ctrl-stick-unit">
          <div class="ctrl-stick" id="stick-right" role="slider" aria-label="右摇杆">
            <div class="ctrl-knob"></div>
          </div>
        </div>

      </div>

    </div>

  </main>

</div>

<script>
(function () {
  'use strict';

  /* 内联脚本已经执行过时，外链兜底脚本直接退出 */
  if (window.__webControlLoaded) {
    return;
  }
  window.__webControlLoaded = true;

  var state = { lx: 0, ly: 0, rx: 0, ry: 0 };
  var held = {};
  var sendTimer = null;
  var inFlight = false;
  var pending = false;
  var hasPointer = typeof window.PointerEvent !== 'undefined';
  var statusEl = document.getElementById('ctrl-status');
  var readoutEl = document.getElementById('ctrl-readout');

  window.onerror = function (message) {
    if (statusEl) {
      statusEl.textContent = 'JS 错误: ' + message;
    }
    return false;
  };

  function addClass(el, name) {
    if (el.classList) {
      el.classList.add(name);
    } else {
      el.className += ' ' + name;
    }
  }

  function removeClass(el, name) {
    if (el.classList) {
      el.classList.remove(name);
    } else {
      el.className = el.className.replace(
        new RegExp('(^|\\s)' + name + '(\\s|$)', 'g'), ' '
      );
    }
  }

  function heldNames() {
    var names = [];
    for (var key in held) {
      if (held.hasOwnProperty(key) && held[key]) {
        names.push(key);
      }
    }
    return names;
  }

  function buildQuery() {
    var parts = [
      'lx=' + state.lx,
      'ly=' + state.ly,
      'rx=' + state.rx,
      'ry=' + state.ry
    ];
    var names = heldNames();
    for (var i = 0; i < names.length; i++) {
      parts.push(encodeURIComponent(names[i]) + '=1');
    }
    return parts.join('&');
  }

  function updateReadout() {
    if (!readoutEl) {
      return;
    }
    var names = heldNames();
    readoutEl.textContent =
      'L ' + state.lx + ',' + state.ly +
      ' · R ' + state.rx + ',' + state.ry +
      ' · ' + (names.length ? names.join(' ') : '无按键');
  }

  function send() {
    if (inFlight) {
      pending = true;
      return;
    }

    updateReadout();
    inFlight = true;

    var xhr = new XMLHttpRequest();
    var finished = false;

    function finish(ok, text) {
      if (finished) {
        return;
      }
      finished = true;
      inFlight = false;
      if (statusEl) {
        statusEl.textContent = text;
      }
      if (pending) {
        pending = false;
        schedule(false);
      }
    }

    xhr.onreadystatechange = function () {
      if (xhr.readyState === 4) {
        if (xhr.status >= 200 && xhr.status < 300) {
          finish(true, '已连接 ESP32');
        } else {
          finish(false, '发送失败');
        }
      }
    };
    xhr.onerror = function () {
      finish(false, '发送失败，请检查连接');
    };
    xhr.ontimeout = function () {
      finish(false, '发送超时');
    };

    try {
      xhr.open('GET', '/control/state?' + buildQuery(), true);
      xhr.timeout = 3000;
      xhr.send();
    } catch (error) {
      finish(false, '发送失败');
    }
  }

  function schedule(immediate) {
    if (immediate) {
      if (sendTimer) {
        clearTimeout(sendTimer);
        sendTimer = null;
      }
      send();
      return;
    }

    if (sendTimer) {
      return;
    }
    sendTimer = setTimeout(function () {
      sendTimer = null;
      send();
    }, 50);
  }

  function sendNeutral() {
    var body = 'lx=0&ly=0&rx=0&ry=0';
    if (navigator.sendBeacon && window.Blob) {
      try {
        navigator.sendBeacon(
          '/control/state',
          new Blob([body], { type: 'application/x-www-form-urlencoded' })
        );
      } catch (error) {}
    }
  }

  /* ---------------- 按键 ---------------- */

  function bindButton(el) {
    var name = el.getAttribute('data-btn');

    function press(event) {
      if (event) {
        event.preventDefault();
        if (event.pointerId !== undefined && el.setPointerCapture) {
          try { el.setPointerCapture(event.pointerId); } catch (e) {}
        }
      }
      if (!held[name]) {
        held[name] = 1;
        addClass(el, 'active');
        updateReadout();
        schedule(true);
      }
    }

    function release(event) {
      if (event) {
        event.preventDefault();
      }
      if (held[name]) {
        held[name] = 0;
        removeClass(el, 'active');
        updateReadout();
        schedule(true);
      }
    }

    if (hasPointer) {
      el.addEventListener('pointerdown', press);
      el.addEventListener('pointerup', release);
      el.addEventListener('pointercancel', release);
      el.addEventListener('lostpointercapture', release);
      window.addEventListener('pointerup', release);
      window.addEventListener('pointercancel', release);
    } else {
      el.addEventListener('mousedown', press);
      el.addEventListener('touchstart', press, { passive: false });
      window.addEventListener('mouseup', release);
      window.addEventListener('touchend', release);
      window.addEventListener('touchcancel', release);
    }

    el.addEventListener('keydown', function (event) {
      if ((event.key === ' ' || event.key === 'Enter') && !event.repeat) {
        press(null);
      }
    });
    el.addEventListener('keyup', function (event) {
      if (event.key === ' ' || event.key === 'Enter') {
        release(null);
      }
    });
  }

  var buttonList = document.querySelectorAll('[data-btn]');
  for (var b = 0; b < buttonList.length; b++) {
    bindButton(buttonList[b]);
  }

  /* ---------------- 摇杆 ---------------- */

  function setupStick(element, xKey, yKey) {
    if (!element) {
      return;
    }

    var knob = element.querySelector('.ctrl-knob');
    var activePointer = null;
    var mouseActive = false;
    var activeTouch = null;

    function setFromPoint(clientX, clientY) {
      var rect = element.getBoundingClientRect();
      var centerX = rect.left + rect.width / 2;
      var centerY = rect.top + rect.height / 2;
      var dx = clientX - centerX;
      var dy = clientY - centerY;
      var max = Math.max(1, rect.width / 2 - knob.offsetWidth / 2);
      var distance = Math.sqrt(dx * dx + dy * dy);

      if (distance > max) {
        dx = dx / distance * max;
        dy = dy / distance * max;
      }

      knob.style.left = 'calc(50% + ' + dx + 'px)';
      knob.style.top = 'calc(50% + ' + dy + 'px)';

      state[xKey] = Math.round(dx / max * 100);
      state[yKey] = Math.round(-dy / max * 100);
      updateReadout();
      schedule(false);
    }

    function resetStick() {
      knob.style.left = '50%';
      knob.style.top = '50%';
      state[xKey] = 0;
      state[yKey] = 0;
      updateReadout();
      schedule(true);
    }

    function findTouch(touchList, id) {
      for (var i = 0; i < touchList.length; i++) {
        if (touchList[i].identifier === id) {
          return touchList[i];
        }
      }
      return null;
    }

    if (hasPointer) {
      var start = function (event) {
        event.preventDefault();
        activePointer = event.pointerId;
        if (element.setPointerCapture) {
          try { element.setPointerCapture(event.pointerId); } catch (e) {}
        }
        setFromPoint(event.clientX, event.clientY);
        schedule(true);
      };
      var move = function (event) {
        if (event.pointerId !== activePointer) {
          return;
        }
        event.preventDefault();
        setFromPoint(event.clientX, event.clientY);
      };
      var end = function (event) {
        if (event.pointerId !== activePointer) {
          return;
        }
        activePointer = null;
        resetStick();
      };

      element.addEventListener('pointerdown', start);
      element.addEventListener('pointermove', move);
      element.addEventListener('pointerup', end);
      element.addEventListener('pointercancel', end);
      element.addEventListener('lostpointercapture', end);
      window.addEventListener('pointermove', move);
      window.addEventListener('pointerup', end);
      window.addEventListener('pointercancel', end);
    } else {
      element.addEventListener('mousedown', function (event) {
        if (event.button !== 0) {
          return;
        }
        event.preventDefault();
        mouseActive = true;
        setFromPoint(event.clientX, event.clientY);
        schedule(true);
      });
      window.addEventListener('mousemove', function (event) {
        if (!mouseActive) {
          return;
        }
        event.preventDefault();
        setFromPoint(event.clientX, event.clientY);
      });
      window.addEventListener('mouseup', function () {
        if (!mouseActive) {
          return;
        }
        mouseActive = false;
        resetStick();
      });

      element.addEventListener('touchstart', function (event) {
        if (activeTouch !== null) {
          return;
        }
        event.preventDefault();
        var touch = event.changedTouches[0];
        activeTouch = touch.identifier;
        setFromPoint(touch.clientX, touch.clientY);
        schedule(true);
      }, { passive: false });

      window.addEventListener('touchmove', function (event) {
        if (activeTouch === null) {
          return;
        }
        var touch = findTouch(event.touches, activeTouch);
        if (!touch) {
          return;
        }
        event.preventDefault();
        setFromPoint(touch.clientX, touch.clientY);
      }, { passive: false });

      function endTouch(event) {
        if (activeTouch === null) {
          return;
        }
        if (!findTouch(event.changedTouches, activeTouch)) {
          return;
        }
        activeTouch = null;
        resetStick();
      }
      window.addEventListener('touchend', endTouch);
      window.addEventListener('touchcancel', endTouch);
    }
  }

  setupStick(document.getElementById('stick-left'), 'lx', 'ly');
  setupStick(document.getElementById('stick-right'), 'rx', 'ry');

  /* ---------------- 重置 ---------------- */

  function resetAll() {
    held = {};
    var buttons = document.querySelectorAll('.ctrl-pad button');
    for (var i = 0; i < buttons.length; i++) {
      removeClass(buttons[i], 'active');
    }
    var knobs = document.querySelectorAll('.ctrl-knob');
    for (var k = 0; k < knobs.length; k++) {
      knobs[k].style.left = '50%';
      knobs[k].style.top = '50%';
    }
    state.lx = 0;
    state.ly = 0;
    state.rx = 0;
    state.ry = 0;
    updateReadout();
    schedule(true);
  }

  var resetButton = document.getElementById('ctrl-reset');
  if (resetButton) {
    resetButton.addEventListener('click', resetAll);
  }

  var pad = document.querySelector('.ctrl-pad');
  if (pad) {
    pad.addEventListener('contextmenu', function (event) {
      event.preventDefault();
    });
  }

  updateReadout();

  /* 持续发送当前状态：即使某条松开指令丢失，下一帧也会补发归零 */
  setInterval(function () {
    send();
  }, 50);

  /* ---------------- 系统状态：电量 / 音量 / 播放状态 ---------------- */
  var sysinfoEl = document.getElementById('ctrl-sysinfo');
  var playinfoEl = document.getElementById('ctrl-playinfo');
  var sysinfoBusy = false;

  function updateSysinfo() {
    if (!sysinfoEl || sysinfoBusy) {
      return;
    }

    sysinfoBusy = true;

    var xhr = new XMLHttpRequest();

    function done() {
      sysinfoBusy = false;
    }

    xhr.onreadystatechange = function () {
      if (xhr.readyState !== 4) {
        return;
      }
      if (xhr.status >= 200 && xhr.status < 300) {
        var parts = xhr.responseText.split(',');
        if (parts.length >= 2) {
          var b = parseInt(parts[0], 10);
          var v = parseInt(parts[1], 10);
          sysinfoEl.textContent =
            '电量' + (b >= 0 ? b : '--') + '%  音量' +
            (v >= 0 ? v : '--') + '%';

          if (parts.length >= 5 && playinfoEl) {
            var playing = parseInt(parts[2], 10);
            var track = parseInt(parts[3], 10);
            var mode = parseInt(parts[4], 10);
            var files = parts.length >= 6 ? parseInt(parts[5], 10) : 0;
            var modeNames = ['单曲循环', '顺序播放', '随机播放'];

            if (!(mode >= 0 && mode < modeNames.length)) {
              mode = 0;
            }
            if (!(track >= 1)) {
              track = 1;
            }

            playinfoEl.textContent =
              '音频数量:' + (files > 0 ? files : '--') + ' ' +
              (playing ? '播放中' : '已暂停') + '：第' + track + '首  ' +
              modeNames[mode];
          }
        }
      }
      done();
    };
    xhr.onerror = done;
    xhr.ontimeout = done;

    try {
      xhr.open('GET', '/control/status?_=' + Date.now(), true);
      xhr.timeout = 1500;
      xhr.send();
    } catch (error) {
      done();
    }
  }

  updateSysinfo();
  setInterval(updateSysinfo, 1000);

  window.addEventListener('pagehide', function () {
    sendNeutral();
  });
})();

</script>

</body>
</html>
)PAGE_RAW";

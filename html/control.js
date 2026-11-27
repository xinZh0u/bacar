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

#!/usr/bin/env python3
"""生成内置手柄页面头文件（Arduino PROGMEM 方式）。

修改 html/control.html、control.css、control.js、style.css 后运行：
    python3 tools/gen_control_page.py
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML_DIR = ROOT / "html"

html = (HTML_DIR / "control.html").read_text(encoding="utf-8")
control_css = (HTML_DIR / "control.css").read_text(encoding="utf-8")
control_js = (HTML_DIR / "control.js").read_text(encoding="utf-8")
style_css = (HTML_DIR / "style.css").read_text(encoding="utf-8")

# 所有样式和脚本都内联，页面不依赖额外请求
html = html.replace(
    '<link rel="stylesheet" href="/style.css">',
    "<style>\n" + style_css + "\n</style>",
)
html = html.replace("<!--CONTROL_CSS-->", control_css)
html = html.replace("<!--CONTROL_JS-->", control_js)

if ")PAGE_RAW\"" in html:
    raise SystemExit("页面内容包含 raw literal 结束标记")

header = f"""// 自动生成，请勿直接修改。
// 源文件：html/control.html + control.css + control.js + style.css
// 重新生成：python3 tools/gen_control_page.py
#pragma once

#include <Arduino.h>

static const char WEB_CONTROL_PAGE[] PROGMEM = R"PAGE_RAW(
{html})PAGE_RAW";
"""

out = ROOT / "src" / "web_control_page.h"
out.write_text(header, encoding="utf-8")
print(f"generated {out} ({len(header)} bytes)")

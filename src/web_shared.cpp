#include "web_shared.h"

String web_html_escape(const String &input)
{
    String output;
    output.reserve(input.length() + 16);
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        switch (c) {
            case '&':  output += "&amp;";  break;
            case '<':  output += "&lt;";   break;
            case '>':  output += "&gt;";   break;
            case '"':  output += "&quot;"; break;
            case '\'': output += "&#39;";  break;
            default:   output += c;        break;
        }
    }
    return output;
}

String web_url_encode(const String &input)
{
    static const char hex[] = "0123456789ABCDEF";
    String output;
    output.reserve(input.length() * 3);
    for (size_t i = 0; i < input.length(); i++) {
        uint8_t c = (uint8_t)input[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            output += (char)c;
        } else {
            output += '%';
            output += hex[c >> 4];
            output += hex[c & 0x0F];
        }
    }
    return output;
}

String web_load_asset(const uint8_t *start, const uint8_t *end)
{
    return String((const char *)start, (size_t)(end - start));
}

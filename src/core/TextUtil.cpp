#include "core/TextUtil.h"

string TextUtil::intToText(int value)
{
    if (value == 0)
    {
        return "0";
    }
    bool negative = value < 0;
    // build the digits in reverse, then flip the string
    string digits = "";
    long long v = value;
    if (negative)
    {
        v = -v;
    }
    while (v > 0)
    {
        char c = (char)('0' + (int)(v % 10));
        digits = digits + c;
        v = v / 10;
    }
    string result = "";
    if (negative)
    {
        result = "-";
    }
    for (int i = (int)digits.length() - 1; i >= 0; i--)
    {
        result = result + digits[i];
    }
    return result;
}

int TextUtil::textToInt(const string& text, bool& ok)
{
    ok = false;
    int i = 0;
    int n = (int)text.length();
    bool negative = false;
    if (i < n && (text[i] == '-' || text[i] == '+'))
    {
        negative = text[i] == '-';
        i++;
    }
    long long value = 0;
    bool any = false;
    while (i < n && text[i] >= '0' && text[i] <= '9')
    {
        value = value * 10 + (text[i] - '0');
        any = true;
        i++;
        if (value > 2000000000)
        {
            return 0;   // too large, treat as invalid
        }
    }
    if (!any || i != n)
    {
        return 0;
    }
    ok = true;
    if (negative)
    {
        value = -value;
    }
    return (int)value;
}

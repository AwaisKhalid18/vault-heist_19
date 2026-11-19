#pragma once

#include <string>

using namespace std;

// Tiny text helpers, written by hand because <algorithm> and std::to_string
// style shortcuts are off-limits for everything we can easily do ourselves.
class TextUtil
{
public:
    static string intToText(int value);
    static int textToInt(const string& text, bool& ok);
};

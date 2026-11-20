#pragma once

#include <string>
#include "game/LevelDef.h"

using namespace std;

// Our own small JSON reader. It only understands what our level files use:
// one flat object with string values, integer values, and arrays of
// integers or strings. That keeps it short enough to explain line by line.
class LevelLoader
{
public:
    static bool load(const string& path, LevelDef& out);

    // Reads levels.txt (one level file path per line, '#' starts a comment).
    static int loadLevelList(const string& path, string outPaths[], int maxCount);

private:
    static void skipSpaces(const string& text, int& pos);
    static bool readString(const string& text, int& pos, string& out);
    static bool readInt(const string& text, int& pos, int& out);
    static void applyPair(const string& key, const string& text, int& pos, LevelDef& def, bool& ok);
};

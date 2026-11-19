#include "game/LevelLoader.h"
#include <fstream>
#include <sstream>

void LevelLoader::skipSpaces(const string& text, int& pos)
{
    int n = (int)text.length();
    while (pos < n)
    {
        char c = text[pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',')
        {
            pos++;
        }
        else
        {
            break;
        }
    }
}

// Reads a double-quoted string starting at pos. Supports \" and \\ escapes.
bool LevelLoader::readString(const string& text, int& pos, string& out)
{
    int n = (int)text.length();
    skipSpaces(text, pos);
    if (pos >= n || text[pos] != '"')
    {
        return false;
    }
    pos++;
    out = "";
    while (pos < n && text[pos] != '"')
    {
        if (text[pos] == '\\' && pos + 1 < n)
        {
            pos++;   // take the escaped character literally
        }
        out = out + text[pos];
        pos++;
    }
    if (pos >= n)
    {
        return false;   // missing closing quote
    }
    pos++;
    return true;
}

bool LevelLoader::readInt(const string& text, int& pos, int& out)
{
    int n = (int)text.length();
    skipSpaces(text, pos);
    bool negative = false;
    if (pos < n && text[pos] == '-')
    {
        negative = true;
        pos++;
    }
    bool any = false;
    long long value = 0;
    while (pos < n && text[pos] >= '0' && text[pos] <= '9')
    {
        value = value * 10 + (text[pos] - '0');
        any = true;
        pos++;
    }
    if (!any)
    {
        return false;
    }
    if (negative)
    {
        value = -value;
    }
    out = (int)value;
    return true;
}

// Reads the value after "key": and stores it in the matching LevelDef field.
void LevelLoader::applyPair(const string& key, const string& text, int& pos, LevelDef& def, bool& ok)
{
    int n = (int)text.length();
    skipSpaces(text, pos);
    if (pos >= n)
    {
        ok = false;
        return;
    }

    if (text[pos] == '"')
    {
        string value;
        if (!readString(text, pos, value))
        {
            ok = false;
            return;
        }
        if (key == "id") { def.id = value; }
        else if (key == "title") { def.title = value; }
        else if (key == "mission") { def.mission = value; }
        else if (key == "concept") { def.concept = value; }
        else if (key == "briefing") { def.briefing = value; }
        else if (key == "type") { def.type = value; }
        else if (key == "generator") { def.generator = value; }
        // unknown string keys are ignored on purpose
        return;
    }

    if (text[pos] == '[')
    {
        pos++;
        // an array of ints (keys) or strings (hints)
        while (pos < n)
        {
            skipSpaces(text, pos);
            if (pos < n && text[pos] == ']')
            {
                pos++;
                return;
            }
            if (pos < n && text[pos] == '"')
            {
                string item;
                if (!readString(text, pos, item))
                {
                    ok = false;
                    return;
                }
                if (key == "hints" && def.hintCount < Limits::MAX_HINTS)
                {
                    def.hints[def.hintCount] = item;
                    def.hintCount++;
                }
            }
            else
            {
                int item;
                if (!readInt(text, pos, item))
                {
                    ok = false;
                    return;
                }
                if (key == "keys" && def.keyCount < Limits::MAX_LEVEL_KEYS)
                {
                    def.keys[def.keyCount] = item;
                    def.keyCount++;
                }
            }
        }
        ok = false;   // missing closing bracket
        return;
    }

    int value;
    if (!readInt(text, pos, value))
    {
        ok = false;
        return;
    }
    if (key == "act") { def.act = value; }
    else if (key == "seed") { def.seed = value; }
    else if (key == "n") { def.n = value; }
    else if (key == "keyMin") { def.keyMin = value; }
    else if (key == "keyMax") { def.keyMax = value; }
    else if (key == "k") { def.k = value; }
    else if (key == "lo") { def.lo = value; }
    else if (key == "hi") { def.hi = value; }
}

bool LevelLoader::load(const string& path, LevelDef& out)
{
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        return false;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string text = buffer.str();
    file.close();

    out = LevelDef();

    int pos = 0;
    int n = (int)text.length();
    skipSpaces(text, pos);
    if (pos >= n || text[pos] != '{')
    {
        return false;
    }
    pos++;

    bool ok = true;
    while (ok)
    {
        skipSpaces(text, pos);
        if (pos < n && text[pos] == '}')
        {
            return true;
        }
        string key;
        if (!readString(text, pos, key))
        {
            return false;
        }
        skipSpaces(text, pos);
        if (pos >= n || text[pos] != ':')
        {
            return false;
        }
        pos++;
        applyPair(key, text, pos, out, ok);
    }
    return false;
}

int LevelLoader::loadLevelList(const string& path, string outPaths[], int maxCount)
{
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        return 0;
    }
    int count = 0;
    string line;
    while (count < maxCount && getline(file, line))
    {
        // strip trailing carriage return from Windows line endings
        while (line.length() > 0 && (line[line.length() - 1] == '\r' || line[line.length() - 1] == ' '))
        {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0 || line[0] == '#')
        {
            continue;
        }
        outPaths[count] = line;
        count++;
    }
    file.close();
    return count;
}

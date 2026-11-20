#pragma once

#include <string>
#include "core/Limits.h"

using namespace std;

// Everything a level JSON file can specify, with fixed-size arrays.
// `generator` is "random", "sorted" or "explicit" ("explicit" uses `keys`).
// Convention: the first `n` entries of `keys` build the starting tree and
// any remaining entries are scripted later inserts (used by Act 3 rounds).
struct LevelDef
{
    string id;
    int act;
    string title;
    string mission;
    string concept;
    string briefing;
    string type;
    int seed;
    string generator;
    int n;
    int keyMin;
    int keyMax;
    int keys[Limits::MAX_LEVEL_KEYS];
    int keyCount;
    int k;
    int lo;
    int hi;
    string hints[Limits::MAX_HINTS];
    int hintCount;

    LevelDef()
    {
        id = "";
        act = 0;
        title = "";
        mission = "";
        concept = "";
        briefing = "";
        type = "";
        seed = 1;
        generator = "random";
        n = 0;
        keyMin = 10;
        keyMax = 99;
        keyCount = 0;
        k = 0;
        lo = 0;
        hi = 0;
        hintCount = 0;
        for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
        {
            keys[i] = 0;
        }
    }
};

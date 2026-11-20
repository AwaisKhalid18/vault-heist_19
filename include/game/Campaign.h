#pragma once

#include <string>
#include "game/LevelDef.h"

using namespace std;

// The campaign in order: assets/levels/levels.txt plus every level file it
// names. The main menu, the campaign map and the result screen all need the
// same list, so it is loaded once here rather than three times over.
class Campaign
{
public:
    static const int MAX_CAMPAIGN_LEVELS = 32;

    Campaign();

    bool load(const string& listPath);
    int count();
    const LevelDef& get(int index);
    int indexOfId(const string& levelId);

    // How many levels of one act there are, and where that act starts.
    int countInAct(int act);
    int firstOfAct(int act);

private:
    LevelDef defs[MAX_CAMPAIGN_LEVELS];
    int levelCount;
};

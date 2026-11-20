#pragma once

#include "screens/LevelScreenBase.h"

// 2.4 Range Raid: collect every key inside [lo, hi]. Afterwards the model
// replays its pruned range query and greys out the skipped subtrees;
// a clean raid (no misclicks, within the optimal walk) earns a bonus.
class Level24Range : public LevelScreenBase
{
public:
    Level24Range(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onAnimationDone();

private:
    int neededKeys[Limits::MAX_LEVEL_KEYS];
    bool collected[Limits::MAX_LEVEL_KEYS];
    int neededCount;
    int collectedCount;
    bool showingPrune;
};

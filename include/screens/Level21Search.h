#pragma once

#include "screens/LevelScreenBase.h"

// 2.1 Search Heist: click the left or right child at every room until
// you reach the target key. Wrong corridors cost +1 alarm.
class Level21Search : public LevelScreenBase
{
public:
    Level21Search(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);

private:
    int currentId;
    int targetKey;
};

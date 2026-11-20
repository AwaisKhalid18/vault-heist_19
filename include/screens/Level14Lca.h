#pragma once

#include "screens/TreeLevelBase.h"

// Level 1.4 - Find where two escape routes meet.
// The vault marks two rooms. The player clicks the room where the two escape
// routes join - their lowest common ancestor - and then traces the path from
// the entrance down to it, one room at a time.
class Level14Lca : public TreeLevelBase
{
public:
    Level14Lca(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onOperation(int buttonIndex);
    void drawUiOverlay(sf::RenderWindow& window);

private:
    static const int PHASE_PICK_LCA = 0, PHASE_TRACE = 1;
    static const int OP_SHOW_PATHS = 0;

    int phase;
    int markedA, markedB;
    int answerId;
    int path[Limits::MAX_LEVEL_KEYS];
    int pathLength;
    int traced;
    sf::Text lineText;
    bool fontReady;

    int keyOf(int nodeId);
};

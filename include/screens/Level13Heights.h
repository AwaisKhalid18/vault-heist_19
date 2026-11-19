#pragma once

#include "screens/TreeLevelBase.h"

// Level 1.3 - The alarm spreads from the leaves upward.
// The player clicks a room and types its height, bottom-up, the way a
// postorder walk computes it: leaf = 1, and every other room is one more
// than its taller child. When every height is right the level asks for the
// vault's diameter - the longest corridor between any two rooms.
class Level13Heights : public TreeLevelBase
{
public:
    Level13Heights(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onOperation(int buttonIndex);
    void drawUiOverlay(sf::RenderWindow& window);
    string tooltipFor(int nodeId);

private:
    static const int PHASE_HEIGHTS = 0, PHASE_DIAMETER = 1;
    static const int OP_SUBMIT = 0, OP_CLEAR = 1;

    int phase;
    int solvedIds[Limits::MAX_LEVEL_KEYS];
    int solvedCount;
    int roomCount;
    sf::Text lineText;
    bool fontReady;

    bool isSolved(int nodeId);
    bool childrenSolved(TreeNode* node);
    void submitAnswer();
};

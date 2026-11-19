#pragma once

#include "screens/TreeLevelBase.h"

// Level 1.5 - Mirror the vault.
// Clicking a room swaps its two children. Doing that at every room turns the
// vault into its own mirror image, which is exactly what BinaryTree::mirror
// does recursively - and the Auto button runs that one call so the player can
// watch the recursion do the same job.
class Level15Mirror : public TreeLevelBase
{
public:
    Level15Mirror(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onOperation(int buttonIndex);
    void onAnimationDone();
    void drawUiOverlay(sf::RenderWindow& window);

private:
    static const int OP_AUTO = 0, OP_RESET_SHAPE = 1;

    BinaryTree goal;                 // the mirrored vault the player is aiming at
    int startPre[Limits::MAX_LEVEL_KEYS];
    int startIn[Limits::MAX_LEVEL_KEYS];
    int roomCount;
    int swapsMade;
    sf::Text lineText;
    bool fontReady;

    void rebuildStart();
    void checkForWin();
};

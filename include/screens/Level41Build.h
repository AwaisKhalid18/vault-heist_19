#pragma once

#include "screens/HeapLevelBase.h"

// Levels 4.1 (Activate the Vault) and 4.2 (Repair the Security Locks).
//
// Both ask for the same thing in the player's hands: click a parent and one
// of its children to swap them until every parent holds at least as much as
// both of its children. They differ only in their starting array - 4.1 starts
// from an unordered vault and 4.2 from a heap with a few locks knocked out of
// place - and that comes from the level JSON's `generator`, not from code.
//
// The sift and heapify buttons let the player use the real operations instead
// of swapping by hand, charged the alarm those swaps would have cost.
class Level41Build : public HeapLevelBase
{
public:
    Level41Build(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onCellClicked(int index);
    void onOperation(int buttonIndex);
    void onAnimationDone();
    void onUndoRestored();
    void drawUiOverlay(sf::RenderWindow& window);

private:
    static const int OP_SIFT_DOWN = 0, OP_SIFT_UP = 1, OP_HEAPIFY = 2, OP_CLEAR = 3;
    int startValues[Limits::MAX_NODES_GUI];
    int startCount;
    sf::Text hintText;
    bool fontReady;

    void checkForWin();
};

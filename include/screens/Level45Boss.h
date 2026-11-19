#pragma once

#include "screens/HeapLevelBase.h"

// Level 4.5 - Final Vault Boss.
// The whole Heap Sort workflow on a fresh array, in two phases the player has
// to recognise: first build a valid Max Heap, then repeatedly take the root
// until the active heap is empty. The final door opens only when the array is
// in ascending order.
class Level45Boss : public HeapLevelBase
{
public:
    Level45Boss(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onCellClicked(int index);
    void onOperation(int buttonIndex);
    void onAnimationDone();
    void onUndoRestored();
    void drawUiOverlay(sf::RenderWindow& window);

private:
    // phases, as named int constants because the course forbids enums
    static const int PHASE_BUILD = 0, PHASE_SORT = 1, PHASE_DONE = 2;
    static const int OP_BUILD = 0, OP_EXTRACT = 1, OP_HEAP_SORT = 2, OP_CLEAR = 3;

    int startValues[Limits::MAX_NODES_GUI];
    int startCount;
    int phase;
    int buildOptimal;
    sf::Text statusText;
    bool fontReady;

    bool arrayIsAscending();
    void enterSortPhase();
    void checkForWin();
};

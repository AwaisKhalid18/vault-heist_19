#pragma once

#include "screens/HeapLevelBase.h"

// Levels 4.3 (Extract the Greatest Treasure) and 4.4 (Race Against the
// Guards). The heap already satisfies the Max Heap property, so the maximum
// is always the root. The player takes it by clicking index 0; clicking any
// other room costs alarm, because the whole lesson is that a heap tells you
// where the maximum is without searching for it.
//
// 4.3 asks for the `k` largest treasures and 4.4 asks for the whole array
// under a tighter alarm budget. Which one a file is comes from its JSON.
class Level43Extract : public HeapLevelBase
{
public:
    Level43Extract(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onCellClicked(int index);
    void onOperation(int buttonIndex);
    void onAnimationDone();
    void onUndoRestored();
    void drawUiOverlay(sf::RenderWindow& window);

private:
    static const int OP_EXTRACT = 0, OP_HEAPIFY = 1;
    int startValues[Limits::MAX_NODES_GUI];
    int startCount;
    int targetExtractions;
    int extracted;
    sf::Text statusText;
    bool fontReady;

    void doExtraction();
};

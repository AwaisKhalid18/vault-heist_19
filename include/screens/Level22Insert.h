#pragma once

#include "screens/LevelScreenBase.h"
#include "core/Random.h"

// 2.2 Stash Loot: walk the insert path for three new treasures. When the
// correct side of the current room is empty, a ghost door appears; click
// it and the new room grows in at exactly that spot.
class Level22Insert : public LevelScreenBase
{
public:
    Level22Insert(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onLevelEvent(const sf::Event& e);
    void onAnimationDone();
    void drawCanvasOverlay(sf::RenderWindow& window);

private:
    static const int ROUNDS = 3;

    Random rng;
    int round;
    int currentId;
    int newKey;
    bool ghostActive;
    float ghostX, ghostY;
    bool inserting;

    void startRound();
    void refreshGhost();
};

#pragma once

#include "screens/LevelScreenBase.h"

// 3.2 Manual repair with single rotations. The player drags a child room
// onto its parent: a LEFT child performs a right rotation (LL repair), a
// RIGHT child a left rotation (RR repair). Always fix the deepest strain
// first. New deliveries arrive once the vault is calm, straining it again.
class Level32SingleRot : public LevelScreenBase
{
public:
    Level32SingleRot(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onLevelEvent(const sf::Event& e);
    void onAnimationDone();
    void drawCanvasOverlay(sf::RenderWindow& window);
    bool allowUndo();

private:
    int dragSourceId;
    int scriptIndex;       // next scripted key in def.keys
    int pendingInserts;    // scripted inserts still to animate this wave
    bool busy;             // an animation is running

    void refreshViolation();
    void attemptRotation(int childId, int parentId);
    void continueScript();
};

#pragma once

#include "screens/LevelScreenBase.h"

// 3.3 Manual repair with double rotations. For every strained room the
// player first NAMES the case (LR or RL), then performs the two drags in
// order: first at the child, then at the strained room itself.
class Level33DoubleRot : public LevelScreenBase
{
public:
    Level33DoubleRot(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onLevelEvent(const sf::Event& e);
    void onAnimationDone();
    void onDialogChoice(int button);
    void drawCanvasOverlay(sf::RenderWindow& window);
    bool allowUndo();

private:
    int dragSourceId;
    int scriptIndex;
    int pendingInserts;
    bool busy;
    int violationId;       // the strained room of the current round
    int roundCase;         // ROT_LR or ROT_RL
    int repairStep;        // 0 = waiting for case name, 1 = first drag, 2 = second drag

    void refreshViolation();
    void askCase();
    void attemptRotation(int childId, int parentId);
    void continueScript();
};

#pragma once

#include "screens/LevelScreenBase.h"

// 2.5 Boss: The Lazy Architect. Sorted deliveries build the vault into a
// corridor before the player's eyes; then a deep search must finish while
// the alarm capacity shrinks every second step. Ends with the cutscene
// panel explaining why balancing matters.
class Level25Boss : public LevelScreenBase
{
public:
    Level25Boss(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onLevelUpdate(float dt);
    void onAnimationDone();
    void onDialogChoice(int button);
    bool allowUndo();

private:
    static const int PH_BUILD = 0, PH_SEARCH = 1, PH_CUTSCENE = 2;

    int phase;
    int buildIndex;
    int currentId;
    int targetKey;
    int stepsTaken;

    void insertNextDelivery();
    void startSearchPhase();
};

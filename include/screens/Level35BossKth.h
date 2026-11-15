#pragma once

#include "screens/LevelScreenBase.h"
#include "core/Random.h"

// 3.5 Boss: Order-statistic heist. The player descends to the k-th MOST
// valuable treasure using the size badges (O(log n) rooms). Between
// rounds the vault rebalances after a delivery and the player confirms
// the new size labels of the rotated rooms; the validator watches.
class Level35BossKth : public LevelScreenBase
{
public:
    Level35BossKth(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onAnimationDone();
    void onDialogChoice(int button);
    bool allowUndo();

private:
    static const int ROUNDS = 3;
    static const int PH_DESCEND = 0, PH_INSERT = 1, PH_CONFIRM = 2;

    Random rng;
    int round;
    int phase;
    int currentId;
    int targetKey;
    int confirmIds[2];      // rotated rooms whose sizes must be confirmed
    int confirmCount;
    int confirmIndex;
    int correctSize;
    int shownValues[3];     // the three size options the dialog displayed

    void startRound();
    void startRotationPhase();
    void askNextSizeConfirm();
};

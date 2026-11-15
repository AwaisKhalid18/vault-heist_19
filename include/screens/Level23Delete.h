#pragma once

#include "screens/LevelScreenBase.h"

// 2.3 Steal and Collapse: three deletions covering the leaf, one-child
// and two-children cases. The player names the case first, then watches
// the animated collapse (with the successor swap in the two-child case).
class Level23Delete : public LevelScreenBase
{
public:
    Level23Delete(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onAnimationDone();
    void onDialogChoice(int button);
    void onUndoRestored();

private:
    static const int ROUNDS = 3;

    int round;
    int targetKeyNow;
    bool deleting;

    int caseForRound(int r);
    void startRound();
    void askCase();
};

#pragma once

#include "screens/LevelScreenBase.h"

// 3.4 Auto mode and prediction: before each scripted insert the player
// predicts which repair (None / LL / RR / LR / RL) the self-balancing
// vault will perform; the prediction is checked against predictInsert.
class Level34Predict : public LevelScreenBase
{
public:
    Level34Predict(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onAnimationDone();
    void onDialogChoice(int button);
    bool allowUndo();

private:
    int scriptIndex;
    int predictedCase;
    bool inserting;

    void askPrediction();
};

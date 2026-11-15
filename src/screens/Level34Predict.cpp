#include "screens/Level34Predict.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level34Predict::Level34Predict(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    scriptIndex = 0;
    predictedCase = Event::ROT_NONE;
    inserting = false;
}

bool Level34Predict::allowUndo()
{
    return false;   // the script must stay in order
}

void Level34Predict::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, true);   // the vault repairs itself
    showTreeStatic(true);
    badgeMode = TreeRenderer::BADGE_BF;

    // optimal = rooms the perfect insert runs visit, over the whole script
    AVLTree copy;
    copy.copyFrom(tree);
    copy.setAutoRebalance(true);
    EventLog simLog;
    copy.setEventLog(&simLog);
    optimalSteps = 0;
    for (int i = def.n; i < def.keyCount; i++)
    {
        simLog.clear();
        copy.insert(def.keys[i]);
        for (int j = 0; j < simLog.size(); j++)
        {
            if (simLog.get(j).type == Event::VISIT)
            {
                optimalSteps++;
            }
        }
    }

    scriptIndex = def.n;
    inserting = false;
    setObjective("The vault now repairs itself. Predict the rotation each delivery "
                 "will cause, then watch it happen. "
                 + TextUtil::intToText(def.keyCount - def.n) + " deliveries incoming.");
    askPrediction();
}

void Level34Predict::askPrediction()
{
    if (scriptIndex >= def.keyCount)
    {
        win("Every repair predicted. You think like the Architect now.");
        return;
    }
    predictedCase = tree.predictInsert(def.keys[scriptIndex]);
    string labels[5];
    labels[0] = "None";
    labels[1] = "LL";
    labels[2] = "RR";
    labels[3] = "LR";
    labels[4] = "RL";
    openCustomDialog("Prediction time",
                     "Delivery " + TextUtil::intToText(scriptIndex - def.n + 1) + " of "
                     + TextUtil::intToText(def.keyCount - def.n) + ": key "
                     + TextUtil::intToText(def.keys[scriptIndex])
                     + " is about to be inserted.\nWhich repair will the vault perform?",
                     labels, 5);
}

void Level34Predict::onDialogChoice(int button)
{
    // button order matches the ROT_ constants: 0 None, 1 LL, 2 RR, 3 LR, 4 RL
    if (button == predictedCase)
    {
        bonuses += 50;
        flash("Correct prediction! +50");
    }
    else
    {
        penalty(2, 50, "Wrong! It was " + string(
            predictedCase == Event::ROT_NONE ? "no rotation" :
            predictedCase == Event::ROT_LL ? "LL" :
            predictedCase == Event::ROT_RR ? "RR" :
            predictedCase == Event::ROT_LR ? "LR" : "RL") + ".");
    }
    if (state != ST_PLAY)
    {
        return;
    }
    inserting = true;
    beginOperation();
    tree.insert(def.keys[scriptIndex]);
    endOperation(true);
    scriptIndex++;
    addAlarm(countVisits());
}

void Level34Predict::onAnimationDone()
{
    if (!inserting || state != ST_PLAY)
    {
        return;
    }
    inserting = false;
    fitCameraToTree();
    askPrediction();
}

#include "screens/Level24Range.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level24Range::Level24Range(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    neededCount = 0;
    collectedCount = 0;
    showingPrune = false;
}

void Level24Range::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);

    // the pruned query on a silent copy gives the optimal walk and the loot list
    AVLTree copy;
    copy.copyFrom(tree);
    copy.setAutoRebalance(false);
    EventLog simLog;
    copy.setEventLog(&simLog);
    neededCount = copy.rangeQuery(def.lo, def.hi, neededKeys, Limits::MAX_LEVEL_KEYS);
    optimalSteps = 0;
    for (int i = 0; i < simLog.size(); i++)
    {
        if (simLog.get(i).type == Event::VISIT)
        {
            optimalSteps++;
        }
    }

    for (int i = 0; i < neededCount; i++)
    {
        collected[i] = false;
    }
    collectedCount = 0;
    showingPrune = false;
    setObjective("Collect every treasure with a key in [" + TextUtil::intToText(def.lo)
                 + ", " + TextUtil::intToText(def.hi) + "]. "
                 + TextUtil::intToText(neededCount) + " rooms hold wanted loot.");
}

void Level24Range::onNodeClicked(int nodeId)
{
    if (showingPrune)
    {
        return;
    }
    TreeNode* node = tree.findById(nodeId);
    if (node == nullptr)
    {
        return;
    }
    if (node->key < def.lo || node->key > def.hi)
    {
        penalty(1, 0, TextUtil::intToText(node->key) + " is outside the range!");
        return;
    }
    for (int i = 0; i < neededCount; i++)
    {
        if (neededKeys[i] == node->key)
        {
            if (collected[i])
            {
                flash("Already collected");
                return;
            }
            collected[i] = true;
            collectedCount++;
            anim.setStateById(nodeId, AnimationController::STATE_RESULT);
            addAlarm(1);
            break;
        }
    }
    if (state == ST_PLAY && collectedCount == neededCount)
    {
        // show what a pruned range query would have skipped
        showingPrune = true;
        setObjective("All loot secured. Watch which wings the pruned query skips.");
        beginOperation();
        int out[Limits::MAX_LEVEL_KEYS];
        tree.rangeQuery(def.lo, def.hi, out, Limits::MAX_LEVEL_KEYS);
        endOperation(true);
    }
}

void Level24Range::onAnimationDone()
{
    if (!showingPrune || state != ST_PLAY)
    {
        return;
    }
    string message = "Every treasure in the range is yours.";
    if (penaltyCount == 0 && alarmSteps <= optimalSteps)
    {
        bonuses += 100;
        message = message + " Perfect pruning: +100 bonus.";
    }
    win(message);
}

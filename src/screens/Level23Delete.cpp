#include "screens/Level23Delete.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level23Delete::Level23Delete(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    round = 0;
    targetKeyNow = 0;
    deleting = false;
}

int Level23Delete::caseForRound(int r)
{
    if (r == 1)
    {
        return BST::DEL_LEAF;
    }
    if (r == 2)
    {
        return BST::DEL_ONE_CHILD;
    }
    return BST::DEL_TWO_CHILDREN;
}

void Level23Delete::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);

    // optimal: the rooms a perfect run of the three deletions visits
    AVLTree copy;
    copy.copyFrom(tree);
    copy.setAutoRebalance(false);
    EventLog simLog;
    copy.setEventLog(&simLog);
    optimalSteps = 0;
    for (int r = 1; r <= ROUNDS; r++)
    {
        int key = LevelTreeBuilder::pickKeyWithDeleteCase(copy, caseForRound(r));
        simLog.clear();
        copy.remove(key);
        for (int i = 0; i < simLog.size(); i++)
        {
            if (simLog.get(i).type == Event::VISIT)
            {
                optimalSteps++;
            }
        }
    }

    round = 0;
    deleting = false;
    startRound();
}

void Level23Delete::startRound()
{
    round++;
    targetKeyNow = LevelTreeBuilder::pickKeyWithDeleteCase(tree, caseForRound(round));
    if (targetKeyNow == -1)
    {
        // the shrinking tree no longer offers this case; any key still works
        tree.findMin(targetKeyNow);
    }
    TreeNode* node = tree.findByKey(targetKeyNow);
    targetId = node != nullptr ? node->id : -1;
    setObjective("Steal treasure " + TextUtil::intToText(targetKeyNow)
                 + " (" + TextUtil::intToText(round) + " of " + TextUtil::intToText(ROUNDS)
                 + "). First, name the delete case.");
    askCase();
}

void Level23Delete::askCase()
{
    string labels[3];
    labels[0] = "Leaf";
    labels[1] = "One child";
    labels[2] = "Two children";
    openCustomDialog("Which case applies?",
                     "Room " + TextUtil::intToText(targetKeyNow)
                     + " is about to collapse. Count its corridors going down "
                     "and pick the delete case.",
                     labels, 3);
}

void Level23Delete::onDialogChoice(int button)
{
    int chosen = BST::DEL_LEAF;
    if (button == 1)
    {
        chosen = BST::DEL_ONE_CHILD;
    }
    if (button == 2)
    {
        chosen = BST::DEL_TWO_CHILDREN;
    }
    int actual = tree.deleteCaseOf(targetKeyNow);
    if (chosen != actual)
    {
        penalty(1, 0, "Wrong case! Look at the room's children.");
        if (state == ST_PLAY)
        {
            askCase();
        }
        return;
    }
    deleting = true;
    saveUndo();
    beginOperation();
    tree.remove(targetKeyNow);
    endOperation(true);
    addAlarm(countVisits());
    targetId = -1;
}

void Level23Delete::onAnimationDone()
{
    if (!deleting || state != ST_PLAY)
    {
        return;
    }
    deleting = false;
    if (round >= ROUNDS)
    {
        win("Three rooms collapsed cleanly: leaf, one child and two children.");
    }
    else
    {
        startRound();
    }
}

void Level23Delete::onUndoRestored()
{
    // the target may have been deleted; aim the round at the restored tree
    deleting = false;
    round--;
    startRound();
}

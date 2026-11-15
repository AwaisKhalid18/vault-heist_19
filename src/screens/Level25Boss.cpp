#include "screens/Level25Boss.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level25Boss::Level25Boss(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    phase = PH_BUILD;
    buildIndex = 0;
    currentId = -1;
    targetKey = 0;
    stepsTaken = 0;
}

bool Level25Boss::allowUndo()
{
    return phase == PH_SEARCH;
}

void Level25Boss::onLevelStart()
{
    tree.clear();
    tree.setAutoRebalance(false);
    showTreeStatic(true);

    // the whole corridor is the search path, so optimal = n rooms
    optimalSteps = def.n;
    setObjective("The deliveries arrive in sorted order. Watch what the lazy "
                 "architect's vault turns into...");
    phase = PH_BUILD;
    buildIndex = 0;
    stepsTaken = 0;
    anim.setSpeed(2.f);
    insertNextDelivery();
}

void Level25Boss::insertNextDelivery()
{
    buildIndex++;
    beginOperation();
    tree.insert(buildIndex * 10);   // 10, 20, 30, ... always goes right
    endOperation(true);
    anim.setSpeed(3.f);   // the build is a cutscene, so play it briskly
}

// The build phase drives itself off the animation finishing, so never
// leave it paused: that would stop the deliveries for good.
void Level25Boss::onLevelUpdate(float dt)
{
    if (phase == PH_BUILD && state == ST_PLAY && !anim.isPlaying())
    {
        anim.play();
    }
}

void Level25Boss::onAnimationDone()
{
    if (state != ST_PLAY)
    {
        return;
    }
    if (phase == PH_BUILD)
    {
        if (buildIndex < def.n)
        {
            fitCameraToTree();   // keep the growing corridor in view
            insertNextDelivery();
        }
        else
        {
            startSearchPhase();
        }
        return;
    }
}

void Level25Boss::startSearchPhase()
{
    phase = PH_SEARCH;
    anim.setSpeed(1.f);
    fitCameraToTree();
    targetKey = LevelTreeBuilder::pickDeepKey(tree);
    TreeNode* node = tree.findByKey(targetKey);
    targetId = node != nullptr ? node->id : -1;
    currentId = tree.getRoot()->id;
    anim.clearStates();
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    setObjective("Height " + TextUtil::intToText(tree.height()) + " for "
                 + TextUtil::intToText(tree.size()) + " rooms! Reach key "
                 + TextUtil::intToText(targetKey)
                 + " - the capacity shrinks every second step.");
    stepsTaken = 1;
    addAlarm(1);
}

void Level25Boss::onNodeClicked(int nodeId)
{
    if (phase != PH_SEARCH)
    {
        return;
    }
    TreeNode* current = tree.findById(currentId);
    TreeNode* clicked = tree.findById(nodeId);
    if (current == nullptr || clicked == nullptr || clicked->parent != current)
    {
        flash("Only the corridor out of your current room counts");
        return;
    }
    TreeNode* correct = targetKey < current->key ? current->left : current->right;

    stepsTaken++;
    if (stepsTaken % 2 == 0 && capacity > alarmSteps + 1)
    {
        capacity--;    // the guards close in: the meter itself shrinks
        flash("The guards close in... capacity now " + TextUtil::intToText(capacity));
    }

    if (clicked != correct)
    {
        penalty(1, 0, "Wrong corridor!");
        return;
    }
    anim.setStateById(currentId, AnimationController::STATE_VISITED);
    currentId = nodeId;
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    addAlarm(1);
    if (state != ST_PLAY)
    {
        return;
    }
    if (clicked->key == targetKey)
    {
        anim.setStateById(currentId, AnimationController::STATE_RESULT);
        phase = PH_CUTSCENE;
        string labels[1];
        labels[0] = "Lesson learned";
        openCustomDialog("Why balancing matters",
                         "You walked " + TextUtil::intToText(alarmSteps)
                         + " rooms through a corridor of height " + TextUtil::intToText(def.n)
                         + ".\n\nSorted inserts make a BST degenerate: every room hangs on "
                         "the right, so a search costs O(n) steps.\n\nA balanced tree with "
                         "the same rooms would be only about log2(n) floors tall, and the "
                         "same search would cost a handful of steps.\n\nThe Architect's "
                         "Wing ahead keeps itself balanced with AVL rotations.",
                         labels, 1);
    }
}

void Level25Boss::onDialogChoice(int button)
{
    if (phase == PH_CUTSCENE)
    {
        win("You escaped the corridor vault - barely.");
    }
}

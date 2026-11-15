#include "screens/Level21Search.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level21Search::Level21Search(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    currentId = -1;
    targetKey = 0;
}

void Level21Search::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);

    targetKey = LevelTreeBuilder::pickDeepKey(tree);
    optimalSteps = tree.searchPathLength(targetKey);
    setObjective("Reach the room holding key " + TextUtil::intToText(targetKey)
                 + ". Compare at every door: smaller left, larger right.");

    TreeNode* targetNode = tree.findByKey(targetKey);
    targetId = targetNode != nullptr ? targetNode->id : -1;
    currentId = tree.getRoot()->id;
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    addAlarm(1);   // entering the vault visits the root
}

void Level21Search::onNodeClicked(int nodeId)
{
    TreeNode* current = tree.findById(currentId);
    if (current == nullptr || current->key == targetKey)
    {
        return;
    }
    TreeNode* clicked = tree.findById(nodeId);
    if (clicked == nullptr)
    {
        return;
    }
    bool isChild = clicked->parent == current;
    if (!isChild)
    {
        flash("Pick a corridor leading out of your current room");
        return;
    }

    // which corridor would a BST search take?
    TreeNode* correct = targetKey < current->key ? current->left : current->right;
    if (clicked != correct)
    {
        penalty(1, 0, "Wrong corridor! The alarm rises.");
        return;
    }

    anim.setStateById(currentId, AnimationController::STATE_VISITED);
    currentId = nodeId;
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    addAlarm(1);
    if (state != ST_PLAY)
    {
        return;   // that step filled the alarm
    }
    if (clicked->key == targetKey)
    {
        anim.setStateById(currentId, AnimationController::STATE_RESULT);
        win("You found treasure " + TextUtil::intToText(targetKey) + " and slipped away.");
    }
}

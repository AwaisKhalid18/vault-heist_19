#include "screens/Level35BossKth.h"
#include "gui/App.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level35BossKth::Level35BossKth(App& a, const LevelDef& d)
    : LevelScreenBase(a, d), rng(d.seed + 9)
{
    round = 0;
    phase = PH_DESCEND;
    currentId = -1;
    targetKey = 0;
    confirmCount = 0;
    confirmIndex = 0;
    correctSize = 0;
}

bool Level35BossKth::allowUndo()
{
    return false;   // rounds are scripted
}

void Level35BossKth::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, true);
    showTreeStatic(true);
    badgeMode = TreeRenderer::BADGE_SIZE;

    // optimal = the kth descent lengths over all rounds, simulated ahead
    AVLTree copy;
    copy.copyFrom(tree);
    Random simRng(def.seed + 9);
    optimalSteps = 0;
    for (int r = 0; r < ROUNDS; r++)
    {
        int position = copy.size() - def.k + 1;
        int key = 0;
        if (copy.kth(position, key))
        {
            optimalSteps += copy.searchPathLength(key);
        }
        if (r < ROUNDS - 1)
        {
            // the same scripted delivery the live round will make
            int newKey = LevelTreeBuilder::pickMissingKey(copy, def.keyMin, def.keyMax, simRng);
            copy.insert(newKey);
        }
    }

    rng.reseed(def.seed + 9);
    round = 0;
    startRound();
}

void Level35BossKth::startRound()
{
    round++;
    phase = PH_DESCEND;
    int position = tree.size() - def.k + 1;
    tree.kth(position, targetKey);      // silent: no log attached
    TreeNode* node = tree.findByKey(targetKey);
    targetId = -1;                      // the target stays hidden: sizes guide you
    currentId = tree.getRoot()->id;
    anim.clearStates();
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    setObjective("Round " + TextUtil::intToText(round) + " of " + TextUtil::intToText(ROUNDS)
                 + ": steal the " + TextUtil::intToText(def.k) + "-th MOST valuable treasure"
                 + " - the " + TextUtil::intToText(position) + "-th smallest of "
                 + TextUtil::intToText(tree.size()) + ". Use the size badges.");
    addAlarm(1);
    if (node == nullptr)
    {
        lose("The vault glitched.");   // should never happen
    }
}

void Level35BossKth::onNodeClicked(int nodeId)
{
    if (phase != PH_DESCEND || state != ST_PLAY)
    {
        return;
    }
    TreeNode* current = tree.findById(currentId);
    TreeNode* clicked = tree.findById(nodeId);
    if (current == nullptr || clicked == nullptr)
    {
        return;
    }

    // clicking the current room steals its treasure if it is the target
    if (clicked == current)
    {
        if (current->key == targetKey)
        {
            anim.setStateById(currentId, AnimationController::STATE_RESULT);
            if (round >= ROUNDS)
            {
                win("The order-statistic heist is complete. The vault is yours.");
            }
            else
            {
                startRotationPhase();
            }
        }
        else
        {
            penalty(1, 0, "This is not the treasure the contract names!");
        }
        return;
    }

    if (clicked->parent != current)
    {
        flash("Move along the corridors, one room at a time");
        return;
    }

    // the correct corridor follows the kth logic over subtree sizes
    TreeNode* correct = targetKey < current->key ? current->left : current->right;
    if (clicked != correct)
    {
        penalty(1, 0, "The size badges say otherwise!");
        return;
    }
    anim.setStateById(currentId, AnimationController::STATE_VISITED);
    currentId = nodeId;
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    addAlarm(1);
}

void Level35BossKth::startRotationPhase()
{
    phase = PH_INSERT;
    int newKey = LevelTreeBuilder::pickMissingKey(tree, def.keyMin, def.keyMax, rng);
    flash("A new delivery shifts the vault...");

    beginOperation();
    tree.insert(newKey);
    endOperation(true);

    // remember the rotated rooms: their size labels must be confirmed
    confirmCount = 0;
    for (int i = 0; i < opLog.size() && confirmCount < 2; i++)
    {
        if (opLog.get(i).type == Event::ROTATE)
        {
            confirmIds[confirmCount] = opLog.get(i).otherId;   // the new subtree root
            confirmCount++;
        }
    }
    if (confirmCount == 0)
    {
        // no rotation happened: still confirm the root's size
        confirmIds[0] = tree.getRoot()->id;
        confirmCount = 1;
    }
}

void Level35BossKth::onAnimationDone()
{
    if (phase != PH_INSERT || state != ST_PLAY)
    {
        return;
    }
    phase = PH_CONFIRM;
    confirmIndex = 0;
    askNextSizeConfirm();
}

void Level35BossKth::askNextSizeConfirm()
{
    if (confirmIndex >= confirmCount)
    {
        startRound();
        return;
    }
    TreeNode* node = tree.findById(confirmIds[confirmIndex]);
    if (node == nullptr)
    {
        confirmIndex++;
        askNextSizeConfirm();
        return;
    }
    anim.setStateById(node->id, AnimationController::STATE_TARGET);
    correctSize = node->size;

    // three answers: the truth and two believable decoys
    int optionA = correctSize;
    int optionB = correctSize + 1;
    int optionC = correctSize > 1 ? correctSize - 1 : correctSize + 2;
    int order = rng.nextInt(0, 2);
    if (order == 0) { shownValues[0] = optionA; shownValues[1] = optionB; shownValues[2] = optionC; }
    else if (order == 1) { shownValues[0] = optionB; shownValues[1] = optionA; shownValues[2] = optionC; }
    else { shownValues[0] = optionC; shownValues[1] = optionB; shownValues[2] = optionA; }
    string labels[3];
    for (int i = 0; i < 3; i++)
    {
        labels[i] = TextUtil::intToText(shownValues[i]);
    }
    openCustomDialog("Confirm the size label",
                     "The rotation moved room " + TextUtil::intToText(node->key)
                     + ". How many rooms does its subtree hold now?",
                     labels, 3);
}

void Level35BossKth::onDialogChoice(int button)
{
    if (phase != PH_CONFIRM)
    {
        return;
    }
    TreeNode* node = tree.findById(confirmIds[confirmIndex]);
    bool ok = button >= 0 && button < 3 && shownValues[button] == correctSize;
    if (ok)
    {
        flash("Size confirmed.");
        if (node != nullptr)
        {
            anim.setStateById(node->id, AnimationController::STATE_NORMAL);
        }
    }
    else
    {
        penalty(1, 0, "Wrong size! The badge flashes red.");
        if (node != nullptr)
        {
            anim.setStateById(node->id, AnimationController::STATE_VIOLATION);
        }
    }
    if (state != ST_PLAY)
    {
        return;
    }
    confirmIndex++;
    askNextSizeConfirm();
}

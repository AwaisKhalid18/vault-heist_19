#include "screens/Level13Heights.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"

Level13Heights::Level13Heights(App& a, const LevelDef& d) : TreeLevelBase(a, d)
{
    phase = PHASE_HEIGHTS;
    solvedCount = 0;
    roomCount = 0;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        solvedIds[i] = -1;
    }
}

void Level13Heights::onLevelStart()
{
    int pre[Limits::MAX_LEVEL_KEYS];
    int in[Limits::MAX_LEVEL_KEYS];
    roomCount = LevelBuilderB::buildTree(def, tree, pre, in);

    phase = PHASE_HEIGHTS;
    solvedCount = 0;
    selectedId = -1;
    // one answer per room, then one more for the diameter
    optimalSteps = roomCount + 1;

    opPanel.clearButtons();
    opPanel.addButton("Submit answer");
    opPanel.addButton("Clear the box");
    opPanel.setFieldCount(1);
    opPanel.setFieldPlaceholders("height", "");
    opPanel.relayout();

    setObjective("Click a room and type its height. A leaf has height 1; any "
                 "other room is 1 more than its taller child.");
    showTreeStatic(true);
}

bool Level13Heights::isSolved(int nodeId)
{
    for (int i = 0; i < solvedCount; i++)
    {
        if (solvedIds[i] == nodeId)
        {
            return true;
        }
    }
    return false;
}

// A room may only be answered once both of its children have been answered,
// which is what forces the player to work bottom-up like a postorder walk.
bool Level13Heights::childrenSolved(TreeNode* node)
{
    if (node->left != nullptr && !isSolved(node->left->id))
    {
        return false;
    }
    if (node->right != nullptr && !isSolved(node->right->id))
    {
        return false;
    }
    return true;
}

void Level13Heights::onNodeClicked(int nodeId)
{
    if (phase != PHASE_HEIGHTS)
    {
        return;
    }
    if (isSolved(nodeId))
    {
        flash("That room's height is already recorded.");
        return;
    }
    TreeNode* node = nodeById(nodeId);
    if (node == nullptr)
    {
        return;
    }
    if (!childrenSolved(node))
    {
        flash("The alarm spreads upward: answer this room's children first.");
        return;
    }
    selectedId = nodeId;
    opPanel.clearFields();
}

void Level13Heights::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_CLEAR)
    {
        opPanel.clearFields();
        return;
    }
    if (buttonIndex == OP_SUBMIT)
    {
        submitAnswer();
    }
}

void Level13Heights::submitAnswer()
{
    int typed = 0;
    if (!opPanel.getFieldInt(0, typed))
    {
        flash("Type a number in the box first.");
        return;
    }

    if (phase == PHASE_DIAMETER)
    {
        addAlarm(1);
        if (typed == tree.diameter())
        {
            win("Heights mapped and the longest corridor measured at "
                + TextUtil::intToText(typed) + " rooms.");
        }
        else
        {
            penalty(1, 0, "That is not the longest path between two rooms.");
            opPanel.clearFields();
        }
        return;
    }

    if (selectedId == -1)
    {
        flash("Click the room you are answering for first.");
        return;
    }
    TreeNode* node = nodeById(selectedId);
    if (node == nullptr)
    {
        return;
    }

    addAlarm(1);
    if (typed != node->height)
    {
        // spec penalty table: a wrong answer costs 1 alarm
        penalty(1, 0, "Height " + TextUtil::intToText(typed) + " is wrong for that room.");
        opPanel.clearFields();
        return;
    }

    solvedIds[solvedCount] = selectedId;
    solvedCount++;
    anim.setStateById(selectedId, AnimationController::STATE_RESULT);
    selectedId = -1;
    opPanel.clearFields();

    if (solvedCount >= roomCount)
    {
        phase = PHASE_DIAMETER;
        badgeMode = TreeRenderer::BADGE_HEIGHT;     // reveal the badges as a reward
        opPanel.setFieldPlaceholders("diameter", "");
        setObjective("Every height is right. Now report the diameter: the most "
                     "rooms on any path between two rooms.");
        flash("Heights complete. Now measure the longest corridor.");
    }
}

// The tooltip must not give the height away while the player is working it out.
string Level13Heights::tooltipFor(int nodeId)
{
    NodeVisual* v = anim.findById(nodeId);
    if (v == nullptr)
    {
        return "";
    }
    if (isSolved(nodeId) || phase == PHASE_DIAMETER)
    {
        return "Value " + TextUtil::intToText(v->key)
             + "\nHeight " + TextUtil::intToText(v->heightVal)
             + "\nDepth " + TextUtil::intToText(v->depth);
    }
    return "Value " + TextUtil::intToText(v->key)
         + "\nHeight: not recorded yet"
         + "\nDepth " + TextUtil::intToText(v->depth);
}

void Level13Heights::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        lineText.setFont(app.uiFont);
        lineText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    string line;
    if (phase == PHASE_HEIGHTS)
    {
        line = "Heights recorded " + TextUtil::intToText(solvedCount) + " of "
             + TextUtil::intToText(roomCount);
        line = line + (selectedId == -1
            ? "   |   click a room whose children are already done"
            : "   |   room selected - type its height and press Submit");
    }
    else
    {
        line = "All heights correct. Type the vault's diameter and press Submit.";
    }
    lineText.setString(line);
    lineText.setFillColor(phase == PHASE_HEIGHTS ? Theme::dimText() : Theme::gold());
    lineText.setPosition(16.f, 16.f);
    window.draw(lineText);
}

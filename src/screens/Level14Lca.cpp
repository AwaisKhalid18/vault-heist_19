#include "screens/Level14Lca.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/Random.h"
#include "core/TextUtil.h"

Level14Lca::Level14Lca(App& a, const LevelDef& d) : TreeLevelBase(a, d)
{
    phase = PHASE_PICK_LCA;
    markedA = -1;
    markedB = -1;
    answerId = -1;
    pathLength = 0;
    traced = 0;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_LEVEL_KEYS; i++)
    {
        path[i] = -1;
    }
}

void Level14Lca::onLevelStart()
{
    int pre[Limits::MAX_LEVEL_KEYS];
    int in[Limits::MAX_LEVEL_KEYS];
    int n = LevelBuilderB::buildTree(def, tree, pre, in);

    // pick two different rooms with the level's own seed, so the pair is the
    // same every time this level is played
    int ids[Limits::MAX_LEVEL_KEYS];
    tree.setEventLog(nullptr);
    int count = tree.preorderRec(ids, Limits::MAX_LEVEL_KEYS);
    Random rng(def.seed + 17);
    markedA = ids[rng.nextInt(0, count - 1)];
    markedB = markedA;
    int guard = 0;
    while (markedB == markedA && guard < 50)
    {
        markedB = ids[rng.nextInt(0, count - 1)];
        guard++;
    }

    // the answer and the path both come from the model
    answerId = -1;
    tree.lca(markedA, markedB, answerId);
    pathLength = tree.pathTo(answerId, path, Limits::MAX_LEVEL_KEYS);

    phase = PHASE_PICK_LCA;
    traced = 0;
    selectedId = -1;
    targetId = -1;
    optimalSteps = 1 + pathLength;
    if (optimalSteps < 2)
    {
        optimalSteps = 2;
    }
    (void)n;

    opPanel.clearButtons();
    opPanel.addButton("Light both routes");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Click the room where the escape routes from the two marked "
                 "rooms first meet.");
    showTreeStatic(true);
    anim.setStateById(markedA, AnimationController::STATE_TARGET);
    anim.setStateById(markedB, AnimationController::STATE_TARGET);
}

int Level14Lca::keyOf(int nodeId)
{
    NodeVisual* v = anim.findById(nodeId);
    return v != nullptr ? v->key : 0;
}

void Level14Lca::onNodeClicked(int nodeId)
{
    if (phase == PHASE_PICK_LCA)
    {
        addAlarm(1);
        if (nodeId != answerId)
        {
            penalty(1, 0, "The routes do not meet there. Follow both up to the entrance.");
            return;
        }
        phase = PHASE_TRACE;
        traced = 0;
        anim.setStateById(answerId, AnimationController::STATE_RESULT);
        setObjective("Correct. Now trace the path from the entrance down to "
                     "the meeting room, one room at a time.");
        flash("Meeting room found. Trace the route from the entrance.");
        return;
    }

    // tracing: the rooms must be clicked in root-to-meeting-room order
    if (traced >= pathLength)
    {
        return;
    }
    if (nodeId != path[traced])
    {
        penalty(1, 0, "That room is not the next one on the path.");
        return;
    }
    anim.setStateById(nodeId, AnimationController::STATE_VISITED);
    traced++;
    addAlarm(1);
    if (traced >= pathLength)
    {
        win("Both routes meet at room " + TextUtil::intToText(keyOf(answerId))
            + ", " + TextUtil::intToText(pathLength) + " rooms from the entrance.");
    }
}

void Level14Lca::onOperation(int buttonIndex)
{
    if (buttonIndex != OP_SHOW_PATHS)
    {
        return;
    }
    // lighting both routes gives the answer away, so it costs a star
    hintUsed = true;
    int pathA[Limits::MAX_LEVEL_KEYS];
    int pathB[Limits::MAX_LEVEL_KEYS];
    int lengthA = tree.pathTo(markedA, pathA, Limits::MAX_LEVEL_KEYS);
    int lengthB = tree.pathTo(markedB, pathB, Limits::MAX_LEVEL_KEYS);
    for (int i = 0; i < lengthA; i++)
    {
        anim.setStateById(pathA[i], AnimationController::STATE_VISITED);
    }
    for (int i = 0; i < lengthB; i++)
    {
        anim.setStateById(pathB[i], AnimationController::STATE_VISITED);
    }
    anim.setStateById(markedA, AnimationController::STATE_TARGET);
    anim.setStateById(markedB, AnimationController::STATE_TARGET);
    flash("Both routes lit. Where they stop overlapping is the meeting room.");
}

void Level14Lca::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        lineText.setFont(app.uiFont);
        lineText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    string line = "Marked rooms: " + TextUtil::intToText(keyOf(markedA)) + " and "
                + TextUtil::intToText(keyOf(markedB));
    if (phase == PHASE_TRACE)
    {
        line = line + "   |   traced " + TextUtil::intToText(traced) + " of "
             + TextUtil::intToText(pathLength) + " rooms from the entrance";
    }
    else
    {
        line = line + "   |   click the room where their routes meet";
    }
    lineText.setString(line);
    lineText.setFillColor(phase == PHASE_TRACE ? Theme::gold() : Theme::dimText());
    lineText.setPosition(16.f, 16.f);
    window.draw(lineText);
}

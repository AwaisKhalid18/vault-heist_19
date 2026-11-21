#include "screens/Level33DoubleRot.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level33DoubleRot::Level33DoubleRot(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    dragSourceId = -1;
    scriptIndex = 0;
    pendingInserts = 0;
    busy = false;
    violationId = -1;
    roundCase = Event::ROT_NONE;
    repairStep = 0;
}

bool Level33DoubleRot::allowUndo()
{
    return !busy && repairStep != 2;   // not between the two half-rotations
}

void Level33DoubleRot::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);
    badgeMode = TreeRenderer::BADGE_BF;

    // optimal = primitive rotations of a perfect run (2 per double repair)
    AVLTree copy;
    copy.copyFrom(tree);
    copy.setAutoRebalance(false);
    long long rotationsBefore = copy.getRotations();
    int index = def.n;
    int guard = 0;
    while (guard < 200)
    {
        guard++;
        int violation = copy.deepestUnbalancedId();
        if (violation != -1)
        {
            copy.rebalanceAt(violation);
            continue;
        }
        if (index >= def.keyCount)
        {
            break;
        }
        copy.insert(def.keys[index]);
        index++;
        copy.insert(def.keys[index]);
        index++;
    }
    optimalSteps = (int)(copy.getRotations() - rotationsBefore);

    dragSourceId = -1;
    scriptIndex = def.n;
    pendingInserts = 0;
    busy = false;
    repairStep = 0;
    setObjective("Double trouble: name the case first. LR and RL need TWO rotations - "
                 "at the child, then at the strained room. LL and RR need just one.");
    refreshViolation();
}

void Level33DoubleRot::refreshViolation()
{
    violationId = tree.deepestUnbalancedId();
    targetId = violationId;
    if (violationId == -1)
    {
        if (state == ST_PLAY && !busy)
        {
            continueScript();
        }
        return;
    }
    roundCase = tree.caseAt(violationId);
    repairStep = 0;
    askCase();
}

bool Level33DoubleRot::isDoubleCase(int rotCase)
{
    return rotCase == Event::ROT_LR || rotCase == Event::ROT_RL;
}

void Level33DoubleRot::askCase()
{
    TreeNode* node = tree.findById(violationId);
    // All four cases are offered. Finishing the first half of a double
    // rotation leaves the strained room in an LL or RR shape, so limiting
    // the choice to LR and RL would leave the round with no correct answer.
    string labels[4];
    labels[0] = "LL (left-left)";
    labels[1] = "RR (right-right)";
    labels[2] = "LR (left-right)";
    labels[3] = "RL (right-left)";
    openCustomDialog("Name the case",
                     "Room " + TextUtil::intToText(node != nullptr ? node->key : 0)
                     + " is strained. Look at which side is heavy and which way\n"
                     "its heavy child leans. Which rotation repairs it?",
                     labels, 4);
}

void Level33DoubleRot::onDialogChoice(int button)
{
    int chosen = Event::ROT_LL;
    if (button == 1)
    {
        chosen = Event::ROT_RR;
    }
    else if (button == 2)
    {
        chosen = Event::ROT_LR;
    }
    else if (button == 3)
    {
        chosen = Event::ROT_RL;
    }

    if (chosen != roundCase)
    {
        penalty(2, 50, "Wrong case! Check which way the heavy child leans.");
        if (state == ST_PLAY)
        {
            askCase();
        }
        return;
    }
    repairStep = 1;
    if (roundCase == Event::ROT_LR)
    {
        flash("LR: first drag the child's RIGHT child up, then finish at the strained room.");
    }
    else if (roundCase == Event::ROT_RL)
    {
        flash("RL: first drag the child's LEFT child up, then finish at the strained room.");
    }
    else if (roundCase == Event::ROT_LL)
    {
        flash("LL: one rotation - drag the strained room's LEFT child up.");
    }
    else
    {
        flash("RR: one rotation - drag the strained room's RIGHT child up.");
    }
}

void Level33DoubleRot::continueScript()
{
    if (scriptIndex >= def.keyCount)
    {
        if (tree.isBalanced())
        {
            win("Both double rotations mastered. The vault hums in balance.");
        }
        return;
    }
    pendingInserts = 2;
    busy = true;
    flash("New deliveries arrive...");
    beginOperation();
    tree.insert(def.keys[scriptIndex]);
    endOperation(true);
    scriptIndex++;
}

void Level33DoubleRot::attemptRotation(int childId, int parentId)
{
    if (repairStep == 0)
    {
        flash("Name the case first!");
        return;
    }
    TreeNode* child = tree.findById(childId);
    TreeNode* parent = tree.findById(parentId);
    TreeNode* violation = tree.findById(violationId);
    if (child == nullptr || parent == nullptr || violation == nullptr
        || child->parent != parent)
    {
        flash("Drag a room onto its direct parent");
        return;
    }
    bool childIsLeft = parent->left == child;

    // which drag is expected right now?
    TreeNode* expectedParent = nullptr;
    bool expectLeftChild = false;
    if (!isDoubleCase(roundCase))
    {
        // LL and RR are repaired by a single rotation at the strained room
        expectedParent = violation;
        expectLeftChild = roundCase == Event::ROT_LL;   // LL is a right rotation
    }
    else if (repairStep == 1)
    {
        // rotate at the heavy child, against its lean
        expectedParent = roundCase == Event::ROT_LR ? violation->left : violation->right;
        expectLeftChild = roundCase == Event::ROT_RL;   // RL step 1 is a right rotation
    }
    else
    {
        expectedParent = violation;
        expectLeftChild = roundCase == Event::ROT_LR;   // LR step 2 is a right rotation
    }

    bool correct = parent == expectedParent && childIsLeft == expectLeftChild;
    if (!correct)
    {
        penalty(2, 50, "Wrong rotation for step " + TextUtil::intToText(repairStep) + "!");
        return;
    }

    busy = true;
    if (repairStep == 1)
    {
        saveUndo();
    }
    beginOperation();
    if (childIsLeft)
    {
        tree.rotateRightAt(parent->id);
    }
    else
    {
        tree.rotateLeftAt(parent->id);
    }
    endOperation(true);
    addAlarm(1);
}

void Level33DoubleRot::onLevelEvent(const sf::Event& e)
{
    if (busy || state != ST_PLAY)
    {
        dragSourceId = -1;
        return;
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f world = camera.toWorld(app.window,
                                            (float)e.mouseButton.x, (float)e.mouseButton.y);
        dragSourceId = anim.findIdAt(world.x, world.y);
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
    {
        if (dragSourceId != -1)
        {
            sf::Vector2f world = camera.toWorld(app.window,
                                                (float)e.mouseButton.x, (float)e.mouseButton.y);
            int dropId = anim.findIdAt(world.x, world.y);
            if (dropId != -1 && dropId != dragSourceId)
            {
                attemptRotation(dragSourceId, dropId);
            }
        }
        dragSourceId = -1;
    }
}

void Level33DoubleRot::onAnimationDone()
{
    if (state != ST_PLAY)
    {
        return;
    }
    if (pendingInserts > 1)
    {
        pendingInserts--;
        beginOperation();
        tree.insert(def.keys[scriptIndex]);
        endOperation(true);
        scriptIndex++;
        return;
    }
    pendingInserts = 0;
    busy = false;

    // only a double rotation has a second half; LL and RR are already done
    if (repairStep == 1 && isDoubleCase(roundCase))
    {
        repairStep = 2;
        flash("Halfway there: now finish at the strained room.");
        return;
    }
    refreshViolation();
}

void Level33DoubleRot::drawCanvasOverlay(sf::RenderWindow& window)
{
    if (dragSourceId == -1)
    {
        return;
    }
    NodeVisual* source = anim.findById(dragSourceId);
    if (source == nullptr)
    {
        return;
    }
    sf::Vector2f world = worldMouse();
    sf::Vertex line[2];
    line[0] = sf::Vertex(sf::Vector2f(source->x, source->y),
                         Theme::withAlpha(Theme::gold(), 180));
    line[1] = sf::Vertex(world, Theme::withAlpha(Theme::gold(), 180));
    window.draw(line, 2, sf::Lines);
}

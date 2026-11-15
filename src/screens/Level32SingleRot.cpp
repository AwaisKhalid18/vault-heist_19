#include "screens/Level32SingleRot.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level32SingleRot::Level32SingleRot(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    dragSourceId = -1;
    scriptIndex = 0;
    pendingInserts = 0;
    busy = false;
}

bool Level32SingleRot::allowUndo()
{
    return !busy;
}

void Level32SingleRot::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);
    badgeMode = TreeRenderer::BADGE_BF;

    // optimal = rotations a perfect repair run needs across all waves
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
    setObjective("Repair the vault by hand: drag a child room onto its strained "
                 "parent to rotate. Fix the DEEPEST strained room first.");
    refreshViolation();
}

void Level32SingleRot::refreshViolation()
{
    int violation = tree.deepestUnbalancedId();
    targetId = violation;
    if (violation == -1 && state == ST_PLAY && !busy)
    {
        continueScript();
    }
}

void Level32SingleRot::continueScript()
{
    if (scriptIndex >= def.keyCount)
    {
        if (tree.isBalanced())
        {
            win("Every strain repaired with single rotations. The Architect approves.");
        }
        return;
    }
    // a new wave: two scripted deliveries arrive with animation
    pendingInserts = 2;
    busy = true;
    flash("New deliveries arrive...");
    beginOperation();
    tree.insert(def.keys[scriptIndex]);
    endOperation(true);
    scriptIndex++;
}

void Level32SingleRot::attemptRotation(int childId, int parentId)
{
    TreeNode* child = tree.findById(childId);
    TreeNode* parent = tree.findById(parentId);
    if (child == nullptr || parent == nullptr || child->parent != parent)
    {
        flash("Drag a room onto its direct parent");
        return;
    }
    bool childIsLeft = parent->left == child;
    int violationId = tree.deepestUnbalancedId();
    int wanted = violationId != -1 ? tree.caseAt(violationId) : Event::ROT_NONE;

    bool correct = parent->id == violationId
        && ((wanted == Event::ROT_LL && childIsLeft) || (wanted == Event::ROT_RR && !childIsLeft));
    if (!correct)
    {
        penalty(2, 50, "Wrong rotation! That would not relieve the strain.");
        return;
    }

    busy = true;
    saveUndo();
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

void Level32SingleRot::onLevelEvent(const sf::Event& e)
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

void Level32SingleRot::onAnimationDone()
{
    if (state != ST_PLAY)
    {
        return;
    }
    if (pendingInserts > 1)
    {
        // the wave's second delivery
        pendingInserts--;
        beginOperation();
        tree.insert(def.keys[scriptIndex]);
        endOperation(true);
        scriptIndex++;
        return;
    }
    pendingInserts = 0;
    busy = false;
    refreshViolation();
}

void Level32SingleRot::drawCanvasOverlay(sf::RenderWindow& window)
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

    // the faint preview: which primitive rotation this drop would perform
    int overId = anim.findIdAt(world.x, world.y);
    string preview = "";
    TreeNode* child = tree.findById(dragSourceId);
    if (overId != -1 && child != nullptr && child->parent != nullptr
        && child->parent->id == overId)
    {
        preview = child->parent->left == child ? "rotate RIGHT here" : "rotate LEFT here";
    }

    sf::Vertex line[2];
    line[0] = sf::Vertex(sf::Vector2f(source->x, source->y),
                         Theme::withAlpha(Theme::gold(), 180));
    line[1] = sf::Vertex(world, Theme::withAlpha(Theme::gold(), 180));
    window.draw(line, 2, sf::Lines);

    if (preview.length() > 0)
    {
        sf::Text text;
        text.setFont(app.uiFont);
        text.setCharacterSize(Theme::textSize());
        text.setString(preview);
        text.setPosition(world.x + 18.f, world.y - 10.f);
        text.setFillColor(Theme::gold());
        window.draw(text);
    }
}

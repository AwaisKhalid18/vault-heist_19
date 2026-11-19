#include "screens/Level22Insert.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level22Insert::Level22Insert(App& a, const LevelDef& d) : LevelScreenBase(a, d), rng(d.seed + 1)
{
    round = 0;
    currentId = -1;
    newKey = 0;
    ghostActive = false;
    ghostX = 0.f;
    ghostY = 0.f;
    inserting = false;
}

void Level22Insert::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);
    showTreeStatic(true);

    // optimal = rooms walked plus the new room itself, over all 3 inserts,
    // computed on a silent copy that uses the same seeded key sequence
    AVLTree copy;
    copy.copyFrom(tree);
    copy.setAutoRebalance(false);
    Random simRng(def.seed + 1);
    optimalSteps = 0;
    for (int i = 0; i < ROUNDS; i++)
    {
        int key = LevelTreeBuilder::pickMissingKey(copy, def.keyMin, def.keyMax, simRng);
        optimalSteps += copy.searchPathLength(key) + 1;
        copy.insert(key);
    }

    rng.reseed(def.seed + 1);
    round = 0;
    inserting = false;
    startRound();
}

void Level22Insert::startRound()
{
    round++;
    newKey = LevelTreeBuilder::pickMissingKey(tree, def.keyMin, def.keyMax, rng);
    currentId = tree.getRoot()->id;
    anim.clearStates();
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    setObjective("Stash treasure " + TextUtil::intToText(newKey)
                 + " (" + TextUtil::intToText(round) + " of " + TextUtil::intToText(ROUNDS)
                 + "). Walk its insert path and click the ghost door at the empty spot.");
    addAlarm(1);
    refreshGhost();
}

void Level22Insert::refreshGhost()
{
    ghostActive = false;
    TreeNode* current = tree.findById(currentId);
    if (current == nullptr)
    {
        return;
    }
    bool goLeft = newKey < current->key;
    TreeNode* next = goLeft ? current->left : current->right;
    if (next != nullptr)
    {
        return;   // the path continues, no ghost yet
    }
    NodeVisual* v = anim.findById(currentId);
    if (v == nullptr)
    {
        return;
    }
    ghostActive = true;
    ghostX = v->toX + (goLeft ? -Theme::levelGapX() : Theme::levelGapX()) * 0.7f;
    ghostY = v->toY + Theme::levelGapY() * 0.85f;
}

void Level22Insert::onNodeClicked(int nodeId)
{
    if (inserting)
    {
        return;
    }
    TreeNode* current = tree.findById(currentId);
    TreeNode* clicked = tree.findById(nodeId);
    if (current == nullptr || clicked == nullptr || clicked->parent != current)
    {
        flash("Follow the corridors from your current room");
        return;
    }
    TreeNode* correct = newKey < current->key ? current->left : current->right;
    if (clicked != correct)
    {
        penalty(1, 0, "Wrong turn! A search for " + TextUtil::intToText(newKey)
                + " would not go there.");
        return;
    }
    anim.setStateById(currentId, AnimationController::STATE_VISITED);
    currentId = nodeId;
    anim.setStateById(currentId, AnimationController::STATE_CURRENT);
    addAlarm(1);
    refreshGhost();
}

void Level22Insert::onLevelEvent(const sf::Event& e)
{
    if (!ghostActive || inserting || state != ST_PLAY)
    {
        return;
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left
        && !camera.didMove())
    {
        sf::Vector2f world = camera.toWorld(app.window,
                                            (float)e.mouseButton.x, (float)e.mouseButton.y);
        float dx = world.x - ghostX;
        float dy = world.y - ghostY;
        float r = Theme::nodeRadius() + 8.f;
        if (dx * dx + dy * dy <= r * r)
        {
            // the player found the empty spot: run the real insert
            ghostActive = false;
            inserting = true;
            saveUndo();
            beginOperation();
            tree.insert(newKey);
            endOperation(true);
            addAlarm(1);   // the new room itself
        }
    }
}

void Level22Insert::onAnimationDone()
{
    if (!inserting || state != ST_PLAY)
    {
        return;
    }
    inserting = false;
    if (round >= ROUNDS)
    {
        win("All three treasures are stashed exactly where a BST insert puts them.");
    }
    else
    {
        startRound();
    }
}

void Level22Insert::drawCanvasOverlay(sf::RenderWindow& window)
{
    if (!ghostActive || state != ST_PLAY)
    {
        return;
    }
    // dashed-looking ghost door: translucent ring + the key inside
    sf::CircleShape ghost(Theme::nodeRadius());
    ghost.setOrigin(Theme::nodeRadius(), Theme::nodeRadius());
    ghost.setPosition(ghostX, ghostY);
    ghost.setFillColor(Theme::withAlpha(Theme::gold(), 30));
    ghost.setOutlineThickness(2.f);
    ghost.setOutlineColor(Theme::withAlpha(Theme::gold(), 170));
    window.draw(ghost);

    sf::Text keyText;
    keyText.setFont(app.monoFont);
    keyText.setCharacterSize(15);
    keyText.setString(TextUtil::intToText(newKey));
    sf::FloatRect b = keyText.getLocalBounds();
    keyText.setPosition(ghostX - b.width / 2.f - b.left, ghostY - b.height / 2.f - b.top);
    keyText.setFillColor(Theme::withAlpha(Theme::gold(), 200));
    window.draw(keyText);
}

#include "screens/Level31Badges.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelTreeBuilder.h"
#include "core/TextUtil.h"

Level31Badges::Level31Badges(App& a, const LevelDef& d) : LevelScreenBase(a, d)
{
    submitReady = false;
    selectedCount = 0;
    revealed = false;
}

void Level31Badges::onLevelStart()
{
    LevelTreeBuilder::build(def, tree, false);   // auto-rebalance off: strain stays
    showTreeStatic(true);
    badgeMode = TreeRenderer::BADGE_HEIGHT;      // heights only, no BF colours yet

    int unbalanced[Limits::MAX_NODES_GUI];
    optimalSteps = tree.unbalancedIds(unbalanced, Limits::MAX_NODES_GUI);
    selectedCount = 0;
    revealed = false;
    setObjective("Badges show each room's HEIGHT. Click every room whose balance "
                 "factor is outside [-1, 1], then press Submit. "
                 + TextUtil::intToText(optimalSteps) + " rooms are strained.");
}

bool Level31Badges::isSelected(int nodeId)
{
    for (int i = 0; i < selectedCount; i++)
    {
        if (selectedIds[i] == nodeId)
        {
            return true;
        }
    }
    return false;
}

void Level31Badges::toggle(int nodeId)
{
    for (int i = 0; i < selectedCount; i++)
    {
        if (selectedIds[i] == nodeId)
        {
            selectedIds[i] = selectedIds[selectedCount - 1];
            selectedCount--;
            anim.setStateById(nodeId, AnimationController::STATE_NORMAL);
            return;
        }
    }
    selectedIds[selectedCount] = nodeId;
    selectedCount++;
    anim.setStateById(nodeId, AnimationController::STATE_SELECTED);
}

void Level31Badges::onNodeClicked(int nodeId)
{
    if (revealed)
    {
        return;
    }
    toggle(nodeId);
    addAlarm(1);
}

void Level31Badges::submit()
{
    if (revealed || state != ST_PLAY)
    {
        return;
    }
    int unbalanced[Limits::MAX_NODES_GUI];
    int violationCount = tree.unbalancedIds(unbalanced, Limits::MAX_NODES_GUI);

    int wrong = 0;
    for (int i = 0; i < selectedCount; i++)   // selected but actually fine
    {
        bool isViolation = false;
        for (int j = 0; j < violationCount; j++)
        {
            if (unbalanced[j] == selectedIds[i])
            {
                isViolation = true;
            }
        }
        if (!isViolation)
        {
            wrong++;
        }
    }
    for (int j = 0; j < violationCount; j++)   // violations the player missed
    {
        if (!isSelected(unbalanced[j]))
        {
            wrong++;
        }
    }

    if (wrong > 0)
    {
        penalty(wrong, 0, TextUtil::intToText(wrong) + " room(s) misjudged - look again!");
        return;
    }

    // reveal: colour BF badges and mark the real violations
    revealed = true;
    badgeMode = TreeRenderer::BADGE_BF;
    anim.clearStates();
    for (int j = 0; j < violationCount; j++)
    {
        anim.setStateById(unbalanced[j], AnimationController::STATE_VIOLATION);
    }
    win("Perfect reading: you spotted every strained room.");
}

void Level31Badges::onLevelEvent(const sf::Event& e)
{
    if (submitReady && submitButton.handleEvent(e))
    {
        submit();
    }
}

void Level31Badges::onLevelUpdate(float dt)
{
    if (!submitReady)
    {
        submitButton.init(&app.uiFont, "Submit", rightX + 14.f, subPanelY + 16.f,
                          rightW - 28.f, 44.f);
        submitButton.setAccent(true);
        submitReady = true;
    }
    submitButton.update(dt, mouseX, mouseY);
}

void Level31Badges::drawUiOverlay(sf::RenderWindow& window)
{
    if (submitReady)
    {
        submitButton.draw(window);
    }
}

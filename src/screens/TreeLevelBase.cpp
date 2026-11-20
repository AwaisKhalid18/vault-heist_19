#include "screens/TreeLevelBase.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"
#include "core/NodeStack.h"

TreeLevelBase::TreeLevelBase(App& a, const LevelDef& d) : app(a), def(d)
{
    state = ST_BRIEFING;
    dialogKind = DLG_NONE;
    alarmSteps = 0;
    capacity = 10;
    optimalSteps = 1;
    penaltyCount = 0;
    penaltyPoints = 0;
    bonuses = 0;
    hintUsed = false;
    hintIndex = 0;
    badgeMode = TreeRenderer::BADGE_NONE;
    hoveredId = -1;
    selectedId = -1;
    targetId = -1;
    mouseX = 0.f;
    mouseY = 0.f;
    animWasFinished = true;
    showTooltips = true;
    rightX = 0.f;
    rightW = 0.f;
    flashTimer = 0.f;
    objective = "";
    briefingPending = false;
    pendingDialog = false;
    pendingCount = 0;
}

TreeLevelBase::~TreeLevelBase()
{
}

void TreeLevelBase::onEnter()
{
    renderer.init(&app.monoFont, &app.uiFont);
    hud.init(&app.titleFont, &app.uiFont);
    toolbar.init(&app.uiFont);
    logPanel.init(&app.titleFont, &app.monoFont);
    opPanel.init(&app.titleFont, &app.uiFont, "Blueprint Tools");
    hintPanel.init(&app.titleFont, &app.uiFont);
    dialog.init(&app.titleFont, &app.uiFont);
    tooltip.init(&app.uiFont);
    flashText.setFont(app.uiFont);
    flashText.setCharacterSize(Theme::headingSize());
    layoutWidgets();

    if (state == ST_BRIEFING)
    {
        briefingPending = true;
        pendingDialog = false;
        restartLevel();
        dialog.open2("Level " + def.id + ": " + def.title,
                     def.briefing + "\n\nMission: " + def.mission,
                     "Begin Heist", "");
        dialogKind = DLG_BRIEFING;
        state = ST_BRIEFING;
    }
}

void TreeLevelBase::restartLevel()
{
    alarmSteps = 0;
    penaltyCount = 0;
    penaltyPoints = 0;
    bonuses = 0;
    hintUsed = false;
    hintIndex = 0;
    hoveredId = -1;
    selectedId = -1;
    targetId = -1;
    state = ST_PLAY;
    hintPanel.hide();
    opLog.clear();
    onLevelStart();
    capacity = ScoreSystem::alarmCapacity(optimalSteps);
    hud.setObjective(objective);
}

void TreeLevelBase::layoutWidgets()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    rightW = 360.f;
    rightX = w - rightW - 16.f;

    camera.setViewport(0.f, 0.f, rightX - 8.f, h - 96.f, w, h);
    hud.setRect(rightX, 16.f, rightW, 196.f);
    float logTop = 224.f;
    float opH = 206.f;
    float opY = h - 96.f - opH - 8.f;
    logPanel.setRect(rightX, logTop, rightW, opY - logTop - 10.f);
    opPanel.setRect(rightX, opY, rightW, opH);
    toolbar.setRect(16.f, h - 82.f, w - 32.f, 66.f);
}

void TreeLevelBase::setObjective(const string& text)
{
    objective = text;
    hud.setObjective(text);
}

void TreeLevelBase::showTreeStatic(bool fitCamera)
{
    opLog.clear();
    opLog.saveTreeShape(tree.getRoot());
    anim.startLog(&opLog, false);
    if (fitCamera)
    {
        fitCameraToTree();
    }
}

void TreeLevelBase::beginOperation()
{
    opLog.clear();
    opLog.saveTreeShape(tree.getRoot());
    tree.setEventLog(&opLog);
}

void TreeLevelBase::endOperation(bool autoPlay)
{
    tree.setEventLog(nullptr);
    anim.setSpeed(toolbar.getSpeed());
    anim.startLog(&opLog, autoPlay);
    animWasFinished = false;
}

TreeNode* TreeLevelBase::nodeById(int id)
{
    NodeStack stack;
    if (tree.getRoot() != nullptr)
    {
        stack.push(tree.getRoot());
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (node->id == id)
        {
            return node;
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }
    return nullptr;
}

void TreeLevelBase::addAlarm(int steps)
{
    if (state != ST_PLAY)
    {
        return;
    }
    alarmSteps += steps;
    if (alarmSteps >= capacity)
    {
        alarmSteps = capacity;
        lose("The alarm is full. The patrol reached the blueprint wing.");
    }
}

void TreeLevelBase::penalty(int alarmPlus, int points, const string& message)
{
    penaltyCount++;
    penaltyPoints += points;
    flash(message);
    addAlarm(alarmPlus);
}

void TreeLevelBase::flash(const string& message)
{
    flashMessage = message;
    flashTimer = 2.2f;
}

LevelOutcome TreeLevelBase::makeOutcome(bool completed)
{
    LevelOutcome o;
    o.alarmSteps = alarmSteps;
    o.optimalSteps = optimalSteps;
    o.capacity = capacity;
    o.penaltyCount = penaltyCount;
    o.penaltyPoints = penaltyPoints;
    o.bonuses = bonuses;
    o.hintUsed = hintUsed;
    o.completed = completed;
    return o;
}

void TreeLevelBase::win(const string& message)
{
    if (state != ST_PLAY)
    {
        return;
    }
    state = ST_WON;
    sf::Vector2u size = app.window.getSize();
    particles.burst((float)size.x / 2.f, (float)size.y / 2.f, 180, def.seed);

    LevelOutcome o = makeOutcome(true);
    int stars = ScoreSystem::computeStars(o);
    int score = ScoreSystem::computeScore(o);
    string starText = "";
    for (int i = 0; i < 3; i++)
    {
        starText = starText + (i < stars ? "*" : "-");
    }
    dialog.open2("Vault Cracked!   [" + starText + "]",
                 message + "\n\nAlarm " + TextUtil::intToText(alarmSteps) + " / "
                 + TextUtil::intToText(capacity) + "   Optimal " + TextUtil::intToText(optimalSteps)
                 + "\nPenalties " + TextUtil::intToText(penaltyCount)
                 + "   Score " + TextUtil::intToText(score),
                 "Continue", "Retry");
    dialogKind = DLG_WIN;
}

void TreeLevelBase::lose(const string& message)
{
    if (state != ST_PLAY)
    {
        return;
    }
    state = ST_LOST;
    dialog.open2("Caught!", message, "Retry", "Give up");
    dialogKind = DLG_LOSE;
}

void TreeLevelBase::fitCameraToTree()
{
    float minX, minY, maxX, maxY;
    anim.getBounds(minX, minY, maxX, maxY);
    camera.fitTo(minX - 40.f, minY - 50.f, maxX + 40.f, maxY + 50.f);
}

void TreeLevelBase::openCustomDialog(const string& title, const string& body,
                                     const string labels[], int count)
{
    if (count > Dialog::MAX_DIALOG_BUTTONS)
    {
        count = Dialog::MAX_DIALOG_BUTTONS;
    }
    if (briefingPending)
    {
        pendingDialog = true;
        pendingTitle = title;
        pendingBody = body;
        pendingCount = count;
        for (int i = 0; i < count; i++)
        {
            pendingLabels[i] = labels[i];
        }
        return;
    }
    dialog.open(title, body, labels, count);
    dialogKind = DLG_CUSTOM;
}

void TreeLevelBase::handleDialogChoice(int button)
{
    int kind = dialogKind;
    if (kind == DLG_BRIEFING)
    {
        dialog.close();
        dialogKind = DLG_NONE;
        state = ST_PLAY;
        briefingPending = false;
        if (pendingDialog)
        {
            pendingDialog = false;
            dialog.open(pendingTitle, pendingBody, pendingLabels, pendingCount);
            dialogKind = DLG_CUSTOM;
        }
        return;
    }
    if (kind == DLG_WIN)
    {
        dialog.close();
        if (button == 0)
        {
            app.levelFinished(def.id, makeOutcome(true));
        }
        else
        {
            dialogKind = DLG_NONE;
            restartLevel();
        }
        return;
    }
    if (kind == DLG_LOSE)
    {
        dialog.close();
        if (button == 0)
        {
            dialogKind = DLG_NONE;
            restartLevel();
        }
        else
        {
            app.levelFinished(def.id, makeOutcome(false));
        }
        return;
    }
    if (kind == DLG_CUSTOM)
    {
        dialog.close();
        dialogKind = DLG_NONE;
        onDialogChoice(button);
    }
}

void TreeLevelBase::handleToolbar(int action)
{
    if (action == Toolbar::ACT_PLAY) { anim.togglePlay(); }
    else if (action == Toolbar::ACT_STEP_BACK) { anim.stepBack(); }
    else if (action == Toolbar::ACT_STEP_FWD) { anim.stepForward(); }
    else if (action == Toolbar::ACT_REPLAY)
    {
        anim.seek(0);
        anim.play();
    }
    else if (action == Toolbar::ACT_RESET)
    {
        restartLevel();
    }
    else if (action == Toolbar::ACT_HINT)
    {
        if (hintPanel.isVisible())
        {
            hintPanel.hide();
        }
        else if (def.hintCount > 0)
        {
            hintUsed = true;
            hintPanel.show(def.hints[hintIndex % def.hintCount]);
            hintIndex++;
        }
    }
    else if (action == Toolbar::ACT_SPEED)
    {
        anim.setSpeed(toolbar.getSpeed());
    }
    else if (action == Toolbar::ACT_SCRUB)
    {
        int target = (int)(toolbar.getScrub() * (float)anim.totalEvents() + 0.5f);
        anim.seek(target);
    }
    else if (action == Toolbar::ACT_FIT)
    {
        fitCameraToTree();
    }
}

void TreeLevelBase::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        mouseX = (float)e.mouseMove.x;
        mouseY = (float)e.mouseMove.y;
    }
    if (e.type == sf::Event::Resized)
    {
        layoutWidgets();
    }

    if (dialog.isOpen())
    {
        int choice = dialog.handleEvent(e);
        if (choice >= 0)
        {
            handleDialogChoice(choice);
        }
        return;
    }

    if (e.type == sf::Event::KeyPressed && !opPanel.anyFieldFocused())
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            app.pop();
            return;
        }
        if (e.key.code == sf::Keyboard::Space) { anim.togglePlay(); return; }
        if (e.key.code == sf::Keyboard::Left) { anim.stepBack(); return; }
        if (e.key.code == sf::Keyboard::Right) { anim.stepForward(); return; }
        if (e.key.code == sf::Keyboard::F) { fitCameraToTree(); return; }
        if (e.key.code == sf::Keyboard::H) { handleToolbar(Toolbar::ACT_HINT); return; }
    }

    int action = toolbar.handleEvent(e);
    if (action != Toolbar::ACT_NONE)
    {
        handleToolbar(action);
        return;
    }
    int button = opPanel.handleEvent(e);
    if (button >= 0)
    {
        if (state == ST_PLAY)
        {
            onOperation(button);
        }
        return;
    }
    if (logPanel.handleEvent(e))
    {
        return;
    }

    if (e.type == sf::Event::MouseWheelScrolled)
    {
        if (camera.viewportContains((float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y))
        {
            camera.zoomAt(e.mouseWheelScroll.delta,
                          (float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
        }
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float px = (float)e.mouseButton.x;
        float py = (float)e.mouseButton.y;
        if (camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            if (anim.findIdAt(world.x, world.y) == -1)
            {
                camera.startDrag(px, py);
            }
        }
    }
    if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
    {
        bool realDrag = camera.isDragging() && camera.didMove();
        camera.endDrag();
        float px = (float)e.mouseButton.x;
        float py = (float)e.mouseButton.y;
        if (!realDrag && state == ST_PLAY && camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            int id = anim.findIdAt(world.x, world.y);
            if (id != -1)
            {
                onNodeClicked(id);
            }
        }
    }
    if (e.type == sf::Event::MouseMoved)
    {
        camera.dragTo((float)e.mouseMove.x, (float)e.mouseMove.y);
    }

    onLevelEvent(e);
}

void TreeLevelBase::update(float dt)
{
    layoutWidgets();
    camera.update(dt);
    anim.update(dt);
    renderer.update(dt);
    particles.update(dt);

    if (camera.viewportContains(mouseX, mouseY) && !dialog.isOpen())
    {
        sf::Vector2f world = camera.toWorld(app.window, mouseX, mouseY);
        hoveredId = anim.findIdAt(world.x, world.y);
    }
    else
    {
        hoveredId = -1;
    }
    string info = hoveredId != -1 && showTooltips ? tooltipFor(hoveredId) : string("");
    if (info.length() > 0)
    {
        tooltip.show(info, mouseX, mouseY);
    }
    else
    {
        tooltip.hide();
    }

    if (state == ST_PLAY && !animWasFinished && anim.isFinished())
    {
        animWasFinished = true;
        onAnimationDone();
    }

    LevelOutcome live = makeOutcome(state == ST_WON);
    hud.setAlarm(alarmSteps, capacity);
    hud.setScoreInfo(ScoreSystem::computeScore(live), alarmSteps, optimalSteps);
    hud.update(dt);
    toolbar.update(dt, mouseX, mouseY, anim.isPlaying(), anim.progress01());
    opPanel.update(dt, mouseX, mouseY);
    hintPanel.update(dt, (float)app.window.getSize().x);
    dialog.update(dt, mouseX, mouseY);
    if (flashTimer > 0.f)
    {
        flashTimer -= dt;
    }

    onLevelUpdate(dt);
}

void TreeLevelBase::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);

    camera.apply(window);
    renderer.draw(window, anim, badgeMode, hoveredId, selectedId, targetId);
    drawCanvasOverlay(window);

    app.useUiView();
    hud.draw(window);
    logPanel.draw(window, anim.getLog(), anim.currentIndex());
    opPanel.draw(window);
    toolbar.draw(window);
    drawUiOverlay(window);
    hintPanel.draw(window);

    if (flashTimer > 0.f && flashMessage.length() > 0)
    {
        flashText.setString(flashMessage);
        sf::FloatRect fb = flashText.getLocalBounds();
        float alpha = flashTimer > 1.f ? 1.f : flashTimer;
        flashText.setFillColor(Theme::withAlpha(Theme::warning(), (int)(alpha * 255.f)));
        flashText.setPosition((rightX - fb.width) / 2.f, 26.f);
        window.draw(flashText);
    }

    particles.draw(window);
    tooltip.draw(window);
    dialog.draw(window);
}

// The default tooltip. Act 1 levels that must not give the answer away
// (heights in 1.3, for instance) override this.
string TreeLevelBase::tooltipFor(int nodeId)
{
    NodeVisual* v = anim.findById(nodeId);
    if (v == nullptr)
    {
        return "";
    }
    return "Value " + TextUtil::intToText(v->key)
         + "\nDepth " + TextUtil::intToText(v->depth)
         + "\nSubtree rooms " + TextUtil::intToText(v->sizeVal);
}

// default empty hooks
void TreeLevelBase::onNodeClicked(int nodeId) {}
void TreeLevelBase::onOperation(int buttonIndex) {}
void TreeLevelBase::onLevelEvent(const sf::Event& e) {}
void TreeLevelBase::onLevelUpdate(float dt) {}
void TreeLevelBase::onAnimationDone() {}
void TreeLevelBase::onDialogChoice(int button) {}
void TreeLevelBase::drawCanvasOverlay(sf::RenderWindow& window) {}
void TreeLevelBase::drawUiOverlay(sf::RenderWindow& window) {}

#include "screens/HeapLevelBase.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "model/HeapValidator.h"
#include "core/TextUtil.h"
#include <iostream>

HeapLevelBase::HeapLevelBase(App& a, const LevelDef& d) : app(a), def(d)
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
    selectedIndex = -1;
    hoveredTreeIndex = -1;
    hoveredCellIndex = -1;
    mouseX = 0.f;
    mouseY = 0.f;
    animWasFinished = true;
    rightX = 0.f;
    rightW = 0.f;
    undoCount = 0;
    undoHeapSize = 0;
    hasUndo = false;
    opStartHeapSize = 0;
    flashTimer = 0.f;
    objective = "";
    arrayY = 0.f;
    arrayH = 96.f;
    briefingPending = false;
    pendingDialog = false;
    pendingCount = 0;
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        undoValues[i] = 0;
    }
}

HeapLevelBase::~HeapLevelBase()
{
}

void HeapLevelBase::onEnter()
{
    heapView.init(&app.monoFont, &app.uiFont);
    arrayView.init(&app.monoFont, &app.uiFont);
    hud.init(&app.titleFont, &app.uiFont);
    toolbar.init(&app.uiFont);
    logPanel.init(&app.titleFont, &app.monoFont);
    opPanel.init(&app.titleFont, &app.uiFont, "Heap Operations");
    hintPanel.init(&app.titleFont, &app.uiFont);
    dialog.init(&app.titleFont, &app.uiFont);
    tooltip.init(&app.uiFont);
    flashText.setFont(app.uiFont);
    flashText.setCharacterSize(Theme::headingSize());
    layoutWidgets();

    if (state == ST_BRIEFING)
    {
        briefingPending = true;    // park any question the level asks on startup
        pendingDialog = false;
        restartLevel();
        dialog.open2("Level " + def.id + ": " + def.title,
                     def.briefing + "\n\nMission: " + def.mission,
                     "Begin Heist", "");
        dialogKind = DLG_BRIEFING;
        state = ST_BRIEFING;
    }
}

void HeapLevelBase::restartLevel()
{
    alarmSteps = 0;
    penaltyCount = 0;
    penaltyPoints = 0;
    bonuses = 0;
    hintUsed = false;
    hintIndex = 0;
    selectedIndex = -1;
    hasUndo = false;
    state = ST_PLAY;
    hintPanel.hide();
    opLog.clear();
    onLevelStart();
    capacity = ScoreSystem::alarmCapacity(optimalSteps);
    hud.setObjective(objective);
}

void HeapLevelBase::layoutWidgets()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    rightW = 360.f;
    rightX = w - rightW - 16.f;

    float canvasW = rightX - 8.f;
    arrayH = 96.f;
    arrayY = h - 96.f - arrayH - 8.f;

    // the tree gets the canvas above the array row
    camera.setViewport(0.f, 0.f, canvasW, arrayY - 8.f, w, h);
    arrayView.setRect(8.f, arrayY, canvasW - 16.f, arrayH);

    hud.setRect(rightX, 16.f, rightW, 196.f);
    float logTop = 224.f;
    float opH = 206.f;
    float opY = h - 96.f - opH - 8.f;
    logPanel.setRect(rightX, logTop, rightW, opY - logTop - 10.f);
    opPanel.setRect(rightX, opY, rightW, opH);
    toolbar.setRect(16.f, h - 82.f, w - 32.f, 66.f);
}

void HeapLevelBase::setObjective(const string& text)
{
    objective = text;
    hud.setObjective(text);
}

void HeapLevelBase::loadHeap(const int values[], int n, int activeHeapSize)
{
    heap.setEventLog(nullptr);
    heap.load(values, n, activeHeapSize);
}

void HeapLevelBase::showHeapStatic(bool fitCamera)
{
    int shown[Limits::MAX_NODES_GUI];
    int n = heap.totalCount();
    if (n > Limits::MAX_NODES_GUI)
    {
        n = Limits::MAX_NODES_GUI;
    }
    for (int i = 0; i < n; i++)
    {
        shown[i] = heap.get(i);
    }
    opLog.clear();
    anim.showStatic(shown, n, heap.getHeapSize());
    if (fitCamera)
    {
        fitCameraToHeap();
    }
}

// Snapshot 0 is the array as it looks BEFORE the operation, which is where
// the animation starts from.
void HeapLevelBase::beginOperation()
{
    int before[Limits::MAX_NODES_GUI];
    int n = heap.totalCount();
    if (n > Limits::MAX_NODES_GUI)
    {
        n = Limits::MAX_NODES_GUI;
    }
    for (int i = 0; i < n; i++)
    {
        before[i] = heap.get(i);
    }
    opLog.clear();
    opLog.saveArray(before, n);
    opStartHeapSize = heap.getHeapSize();
    heap.setEventLog(&opLog);
}

void HeapLevelBase::endOperation(bool autoPlay)
{
    heap.setEventLog(nullptr);
    // Every batch the animator plays ends with a DONE event. The public model
    // operations add their own, but a level that calls siftUp or siftDown
    // directly is only running one step of a larger operation, so close it.
    if (opLog.size() > 0 && opLog.get(opLog.size() - 1).type != Event::DONE)
    {
        Event e;
        e.type = Event::DONE;
        e.heapSize = heap.getHeapSize();
        e.text = "Operation complete";
        opLog.add(e);
    }
    anim.setSpeed(toolbar.getSpeed());
    // the animation starts from the state the operation started in
    anim.start(&opLog, autoPlay, opStartHeapSize);
    animWasFinished = false;
    validateIfWanted();
}

bool HeapLevelBase::heapIsValid()
{
    string error;
    return HeapValidator::checkHeap(heap, error);
}

void HeapLevelBase::validateIfWanted()
{
    if (!app.debugValidate)
    {
        return;
    }
    string error;
    if (!HeapValidator::checkHeap(heap, error))
    {
        cout << "VALIDATOR FAILED in level " << def.id << ": " << error << endl;
    }
}

void HeapLevelBase::addAlarm(int steps)
{
    if (state != ST_PLAY)
    {
        return;
    }
    alarmSteps += steps;
    if (alarmSteps >= capacity)
    {
        alarmSteps = capacity;
        lose("The alarm is full. The guards reached the heap fortress.");
    }
}

void HeapLevelBase::penalty(int alarmPlus, int points, const string& message)
{
    penaltyCount++;
    penaltyPoints += points;
    flash(message);
    addAlarm(alarmPlus);
}

void HeapLevelBase::flash(const string& message)
{
    flashMessage = message;
    flashTimer = 2.2f;
}

LevelOutcome HeapLevelBase::makeOutcome(bool completed)
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

void HeapLevelBase::win(const string& message)
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

void HeapLevelBase::lose(const string& message)
{
    if (state != ST_PLAY)
    {
        return;
    }
    state = ST_LOST;
    dialog.open2("Caught!", message, "Retry", "Give up");
    dialogKind = DLG_LOSE;
}

// Undo for a heap is the array plus the active heap size, exactly what the
// spec's "snapshot before each player action" alternative describes.
void HeapLevelBase::saveUndo()
{
    undoCount = heap.totalCount();
    if (undoCount > Limits::MAX_NODES_GUI)
    {
        undoCount = Limits::MAX_NODES_GUI;
    }
    for (int i = 0; i < undoCount; i++)
    {
        undoValues[i] = heap.get(i);
    }
    undoHeapSize = heap.getHeapSize();
    hasUndo = true;
}

void HeapLevelBase::restoreUndo()
{
    if (!hasUndo || !allowUndo() || state != ST_PLAY)
    {
        return;
    }
    heap.load(undoValues, undoCount, undoHeapSize);
    hasUndo = false;
    showHeapStatic(false);     // snap: no cell may stay in a stale place
    onUndoRestored();
}

void HeapLevelBase::fitCameraToHeap()
{
    float minX, minY, maxX, maxY;
    heapView.getBounds(anim, minX, minY, maxX, maxY);
    camera.fitTo(minX - 40.f, minY - 50.f, maxX + 40.f, maxY + 50.f);
}

void HeapLevelBase::openCustomDialog(const string& title, const string& body,
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

bool HeapLevelBase::allowUndo()
{
    return true;
}

void HeapLevelBase::handleDialogChoice(int button)
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

void HeapLevelBase::handleToolbar(int action)
{
    if (action == Toolbar::ACT_PLAY) { anim.togglePlay(); }
    else if (action == Toolbar::ACT_STEP_BACK) { anim.stepBack(); }
    else if (action == Toolbar::ACT_STEP_FWD) { anim.stepForward(); }
    else if (action == Toolbar::ACT_UNDO) { restoreUndo(); }
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
        fitCameraToHeap();
    }
}

void HeapLevelBase::handleEvent(const sf::Event& e)
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

    // a modal dialog eats all input
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
        if (e.key.code == sf::Keyboard::Z) { restoreUndo(); return; }
        if (e.key.code == sf::Keyboard::F) { fitCameraToHeap(); return; }
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

    // clicking an array cell counts exactly like clicking its tree room
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float px = (float)e.mouseButton.x;
        float py = (float)e.mouseButton.y;
        int cell = arrayView.indexAt(anim, px, py);
        if (cell != -1)
        {
            if (state == ST_PLAY)
            {
                onCellClicked(cell);
            }
            return;
        }
        if (camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            if (heapView.indexAt(anim, world.x, world.y) == -1)
            {
                camera.startDrag(px, py);
            }
        }
    }
    if (e.type == sf::Event::MouseWheelScrolled)
    {
        if (camera.viewportContains((float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y))
        {
            camera.zoomAt(e.mouseWheelScroll.delta,
                          (float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
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
            int index = heapView.indexAt(anim, world.x, world.y);
            if (index != -1)
            {
                onCellClicked(index);
            }
        }
    }
    if (e.type == sf::Event::MouseMoved)
    {
        camera.dragTo((float)e.mouseMove.x, (float)e.mouseMove.y);
    }

    onLevelEvent(e);
}

void HeapLevelBase::update(float dt)
{
    layoutWidgets();
    camera.update(dt);
    anim.update(dt);
    heapView.update(dt);
    particles.update(dt);

    hoveredCellIndex = arrayView.indexAt(anim, mouseX, mouseY);
    hoveredTreeIndex = -1;
    if (hoveredCellIndex == -1 && camera.viewportContains(mouseX, mouseY) && !dialog.isOpen())
    {
        sf::Vector2f world = camera.toWorld(app.window, mouseX, mouseY);
        hoveredTreeIndex = heapView.indexAt(anim, world.x, world.y);
    }

    int hovered = hoveredTreeIndex != -1 ? hoveredTreeIndex : hoveredCellIndex;
    if (hovered != -1)
    {
        int parent = hovered > 0 ? (hovered - 1) / 2 : -1;
        string region = hovered < anim.getHeapSize() ? "active heap" : "sorted region";
        string info = "Value " + TextUtil::intToText(anim.getValue(hovered))
                    + "\nIndex " + TextUtil::intToText(hovered)
                    + "\nParent " + (parent >= 0 ? TextUtil::intToText(parent) : string("none"))
                    + "\nChildren " + TextUtil::intToText(2 * hovered + 1) + ", "
                    + TextUtil::intToText(2 * hovered + 2)
                    + "\nIn the " + region;
        tooltip.show(info, mouseX, mouseY);
    }
    else
    {
        tooltip.hide();
    }

    // only hand this one-shot to the level once play has actually started,
    // or a level chaining off it would lose the signal to the briefing
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

void HeapLevelBase::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);

    camera.apply(window);
    heapView.draw(window, anim, selectedIndex, hoveredTreeIndex);

    app.useUiView();
    arrayView.draw(window, anim, selectedIndex, hoveredCellIndex);
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

bool HeapLevelBase::selectOrSwap(int index)
{
    if (selectedIndex == -1)
    {
        selectedIndex = index;
        return false;
    }
    if (selectedIndex == index)
    {
        selectedIndex = -1;      // clicking the same cell twice just deselects
        return false;
    }
    int first = selectedIndex;
    selectedIndex = -1;
    return performPlayerSwap(first, index);
}

bool HeapLevelBase::performPlayerSwap(int a, int b)
{
    saveUndo();
    beginOperation();
    bool applied = heap.playerSwap(a, b);
    endOperation(true);
    if (!applied)
    {
        hasUndo = false;
        penalty(1, 0, "Only a parent and one of its own children can be swapped.");
        return false;
    }
    addAlarm(1);
    return true;
}

bool HeapLevelBase::extractOnce()
{
    if (heap.getHeapSize() <= 0)
    {
        return false;
    }
    saveUndo();
    beginOperation();
    int taken = 0;
    bool ok = heap.extractMax(taken);
    endOperation(true);
    if (!ok)
    {
        hasUndo = false;
        return false;
    }
    addAlarm(1);
    return true;
}

// default empty hooks
void HeapLevelBase::onCellClicked(int index) {}
void HeapLevelBase::onOperation(int buttonIndex) {}
void HeapLevelBase::onLevelEvent(const sf::Event& e) {}
void HeapLevelBase::onLevelUpdate(float dt) {}
void HeapLevelBase::onAnimationDone() {}
void HeapLevelBase::onUndoRestored() {}
void HeapLevelBase::onDialogChoice(int button) {}
void HeapLevelBase::drawUiOverlay(sf::RenderWindow& window) {}

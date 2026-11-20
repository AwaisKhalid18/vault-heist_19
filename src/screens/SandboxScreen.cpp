#include "screens/SandboxScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "model/TreeValidator.h"
#include "core/Random.h"
#include "core/TextUtil.h"
#include <iostream>

SandboxScreen::SandboxScreen(App& a) : app(a)
{
    tab = TAB_AVL;
    undoCount = 0;
    selectedId = -1;
    hoveredId = -1;
    badgeMode = TreeRenderer::BADGE_BF;
    mouseX = 0.f;
    mouseY = 0.f;
    randomSeed = 1;
    status = "";
    widgetsReady = false;
    bstTree.setAutoRebalance(false);
}

SandboxScreen::~SandboxScreen()
{
    clearUndoStack();
}

AVLTree& SandboxScreen::activeTree()
{
    return tab == TAB_BST ? bstTree : avlTree;
}

bool SandboxScreen::treeTabActive()
{
    return tab == TAB_BST || tab == TAB_AVL;
}

void SandboxScreen::onEnter()
{
    renderer.init(&app.monoFont, &app.uiFont);
    toolbar.init(&app.uiFont);
    logPanel.init(&app.titleFont, &app.monoFont);
    opPanel.init(&app.titleFont, &app.uiFont, "OPERATIONS");
    tooltip.init(&app.uiFont);
    titleText.setFont(app.titleFont);
    titleText.setCharacterSize(Theme::titleSize());
    titleText.setFillColor(Theme::gold());
    titleText.setString("SANDBOX");
    statusText.setFont(app.uiFont);
    statusText.setCharacterSize(Theme::smallSize());
    statusText.setFillColor(Theme::dimText());
    comingSoonText.setFont(app.uiFont);
    comingSoonText.setCharacterSize(Theme::headingSize());
    comingSoonText.setFillColor(Theme::dimText());
    comingSoonText.setString("Binary Tree and Max Heap have their own sandbox: open Sandbox from the main menu.");
    sizeLabel.setFont(app.uiFont);
    sizeLabel.setCharacterSize(Theme::smallSize());
    sizeLabel.setFillColor(Theme::dimText());

    if (avlTree.isEmpty())
    {
        randomSeed = 42;
        randomTree();
    }
    layoutWidgets();
    rebuildOpButtons();
    setTab(tab);
    widgetsReady = true;
}

void SandboxScreen::layoutWidgets()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    float rightW = 360.f;
    float rightX = w - rightW - 16.f;

    camera.setViewport(0.f, 64.f, rightX - 8.f, h - 64.f - 96.f, w, h);

    string names[4] = { "Binary", "BST", "AVL", "MaxHeap" };
    for (int i = 0; i < 4; i++)
    {
        tabButtons[i].init(&app.uiFont, names[i], 220.f + (float)i * 116.f, 14.f, 108.f, 38.f);
        tabButtons[i].setToggled(i == tab);
    }

    opPanel.setRect(rightX, 16.f, rightW, 470.f);
    sizeSlider.init(rightX + 120.f, 500.f, rightW - 160.f);
    logPanel.setRect(rightX, 520.f, rightW, h - 520.f - 100.f);
    toolbar.setRect(16.f, h - 82.f, w - 32.f, 66.f);
}

void SandboxScreen::rebuildOpButtons()
{
    opPanel.clearButtons();
    opPanel.setFieldCount(2);
    opPanel.setFieldPlaceholders("key / k / lo", "hi (range)");
    opPanel.addButton("Insert");          // OP_INSERT
    opPanel.addButton("Delete");          // OP_DELETE
    opPanel.addButton("Search");          // OP_SEARCH
    opPanel.addButton("Range lo-hi");     // OP_RANGE
    opPanel.addButton("Min");             // OP_MIN
    opPanel.addButton("Max");             // OP_MAX
    opPanel.addButton("Successor");       // OP_SUCC
    opPanel.addButton("Predecessor");     // OP_PRED
    opPanel.addButton("K-th smallest");   // OP_KTH
    opPanel.addButton("Rank of key");     // OP_RANK
    opPanel.addButton("RotL @ selected"); // OP_ROT_LEFT
    opPanel.addButton("RotR @ selected"); // OP_ROT_RIGHT
    opPanel.addButton("Auto-rebalance");  // OP_AUTOREBALANCE
    opPanel.addButton("Random tree");     // OP_RANDOM
    opPanel.addButton("Clear");           // OP_CLEAR
    opPanel.addButton("Undo (Z)");        // OP_UNDO
    opPanel.relayout();
}

void SandboxScreen::setTab(int newTab)
{
    tab = newTab;
    for (int i = 0; i < 4; i++)
    {
        tabButtons[i].setToggled(i == tab);
    }
    selectedId = -1;
    clearUndoStack();
    if (treeTabActive())
    {
        badgeMode = tab == TAB_AVL ? TreeRenderer::BADGE_BF : TreeRenderer::BADGE_HEIGHT;
        opPanel.setButtonToggled(OP_AUTOREBALANCE, activeTree().getAutoRebalance());
        opPanel.setButtonEnabled(OP_AUTOREBALANCE, tab == TAB_AVL);
        showStatic(true);
        setStatus(tab == TAB_AVL ? "AVL vault: self-balancing, BF badges."
                                 : "Plain BST vault: no rotations, height badges.");
    }
}

void SandboxScreen::showStatic(bool fit)
{
    opLog.clear();
    opLog.saveTreeShape(activeTree().getRoot());
    anim.startLog(&opLog, false);
    if (fit)
    {
        float minX, minY, maxX, maxY;
        anim.getBounds(minX, minY, maxX, maxY);
        camera.fitTo(minX, minY, maxX, maxY);
    }
}

void SandboxScreen::beginOp()
{
    opLog.clear();
    opLog.saveTreeShape(activeTree().getRoot());
    activeTree().setEventLog(&opLog);
}

void SandboxScreen::endOp()
{
    activeTree().setEventLog(nullptr);
    anim.setSpeed(toolbar.getSpeed());
    anim.startLog(&opLog, true);
    if (app.debugValidate)
    {
        string error;
        bool ok = tab == TAB_AVL && activeTree().getAutoRebalance()
            ? TreeValidator::checkAVL(activeTree(), error)
            : TreeValidator::checkBST(activeTree(), error);
        if (!ok)
        {
            cout << "SANDBOX VALIDATOR FAILED: " << error << endl;
        }
    }
}

void SandboxScreen::saveUndoCopy()
{
    AVLTree* copy = new AVLTree();
    copy->copyFrom(activeTree());
    copy->setAutoRebalance(activeTree().getAutoRebalance());
    if (undoCount == UNDO_DEPTH)
    {
        delete undoStack[0];
        for (int i = 1; i < UNDO_DEPTH; i++)
        {
            undoStack[i - 1] = undoStack[i];
        }
        undoCount--;
    }
    undoStack[undoCount] = copy;
    undoCount++;
}

void SandboxScreen::doUndo()
{
    if (undoCount == 0 || !treeTabActive())
    {
        setStatus("Nothing to undo");
        return;
    }
    undoCount--;
    activeTree().copyFrom(*undoStack[undoCount]);
    activeTree().setAutoRebalance(undoStack[undoCount]->getAutoRebalance());
    delete undoStack[undoCount];
    showStatic(false);   // snap so nothing is left in a wrong place
    setStatus("Undone");
}

void SandboxScreen::clearUndoStack()
{
    for (int i = 0; i < undoCount; i++)
    {
        delete undoStack[i];
    }
    undoCount = 0;
}

void SandboxScreen::randomTree()
{
    AVLTree& tree = activeTree();
    tree.clear();
    int count = 7 + (int)(sizeSlider.getValue() * 56.f + 0.5f);   // 7 .. 63 rooms
    int pool[256];
    for (int i = 0; i < 199; i++)
    {
        pool[i] = i + 1;
    }
    randomSeed++;
    Random rng(randomSeed * 7919);
    rng.shuffle(pool, 199);
    for (int i = 0; i < count && i < 199; i++)
    {
        tree.insert(pool[i]);
    }
    showStatic(true);
    setStatus("Random vault with " + TextUtil::intToText(tree.size()) + " rooms (seed "
              + TextUtil::intToText(randomSeed) + ")");
}

void SandboxScreen::setStatus(const string& text)
{
    status = text;
}

void SandboxScreen::runOperation(int op)
{
    if (!treeTabActive())
    {
        return;
    }
    AVLTree& tree = activeTree();
    int key = 0;
    bool hasKey = opPanel.getFieldInt(0, key);
    int hi = 0;
    bool hasHi = opPanel.getFieldInt(1, hi);
    int result = 0;

    if (op == OP_INSERT)
    {
        if (!hasKey) { setStatus("Type a key first"); return; }
        saveUndoCopy();
        beginOp();
        bool ok = tree.insert(key);
        endOp();
        setStatus(ok ? "Inserted " + TextUtil::intToText(key)
                     : "Duplicate " + TextUtil::intToText(key) + " rejected!");
    }
    else if (op == OP_DELETE)
    {
        if (!hasKey) { setStatus("Type a key first"); return; }
        saveUndoCopy();
        beginOp();
        bool ok = tree.remove(key);
        endOp();
        setStatus(ok ? "Deleted " + TextUtil::intToText(key)
                     : TextUtil::intToText(key) + " is not in the vault");
    }
    else if (op == OP_SEARCH)
    {
        if (!hasKey) { setStatus("Type a key first"); return; }
        beginOp();
        bool ok = tree.search(key);
        endOp();
        setStatus(ok ? "Found " + TextUtil::intToText(key) : "Not found");
    }
    else if (op == OP_RANGE)
    {
        if (!hasKey || !hasHi) { setStatus("Type lo and hi first"); return; }
        beginOp();
        int out[Limits::MAX_NODES_GUI];
        int found = tree.rangeQuery(key, hi, out, Limits::MAX_NODES_GUI);
        endOp();
        setStatus("Range [" + TextUtil::intToText(key) + ", " + TextUtil::intToText(hi)
                  + "] holds " + TextUtil::intToText(found) + " keys");
    }
    else if (op == OP_MIN || op == OP_MAX)
    {
        beginOp();
        bool ok = op == OP_MIN ? tree.findMin(result) : tree.findMax(result);
        endOp();
        setStatus(ok ? (op == OP_MIN ? "Minimum is " : "Maximum is ") + TextUtil::intToText(result)
                     : "The vault is empty");
    }
    else if (op == OP_SUCC || op == OP_PRED)
    {
        if (!hasKey) { setStatus("Type a key first"); return; }
        beginOp();
        bool ok = op == OP_SUCC ? tree.successor(key, result) : tree.predecessor(key, result);
        endOp();
        setStatus(ok ? "Answer: " + TextUtil::intToText(result) : "No such neighbour");
    }
    else if (op == OP_KTH)
    {
        if (!hasKey) { setStatus("Type k first"); return; }
        beginOp();
        bool ok = tab == TAB_AVL ? tree.kth(key, result) : tree.kthSmallest(key, result);
        endOp();
        setStatus(ok ? TextUtil::intToText(key) + "-th smallest is " + TextUtil::intToText(result)
                     : "There is no such k");
    }
    else if (op == OP_RANK)
    {
        if (!hasKey) { setStatus("Type a key first"); return; }
        int rank = tree.rank(key);
        setStatus("rank(" + TextUtil::intToText(key) + ") = " + TextUtil::intToText(rank)
                  + " keys are <= it");
    }
    else if (op == OP_ROT_LEFT || op == OP_ROT_RIGHT)
    {
        if (selectedId == -1) { setStatus("Click a room first to select it"); return; }
        saveUndoCopy();
        beginOp();
        bool ok = op == OP_ROT_LEFT ? tree.rotateLeftAt(selectedId)
                                    : tree.rotateRightAt(selectedId);
        endOp();
        setStatus(ok ? "Rotated" : "That rotation is not possible here");
        if (!ok && undoCount > 0)
        {
            delete undoStack[undoCount - 1];
            undoCount--;
        }
    }
    else if (op == OP_AUTOREBALANCE)
    {
        if (tab != TAB_AVL) { return; }
        bool now = !tree.getAutoRebalance();
        tree.setAutoRebalance(now);
        opPanel.setButtonToggled(OP_AUTOREBALANCE, now);
        setStatus(now ? "Auto-rebalance ON" : "Auto-rebalance OFF (manual mode)");
    }
    else if (op == OP_RANDOM)
    {
        saveUndoCopy();
        randomTree();
    }
    else if (op == OP_CLEAR)
    {
        saveUndoCopy();
        tree.clear();
        showStatic(true);
        setStatus("Vault cleared");
    }
    else if (op == OP_UNDO)
    {
        doUndo();
    }
}

void SandboxScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        mouseX = (float)e.mouseMove.x;
        mouseY = (float)e.mouseMove.y;
    }
    if (e.type == sf::Event::Resized)
    {
        layoutWidgets();
        opPanel.relayout();
    }
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            app.pop();
            return;
        }
        if (!opPanel.anyFieldFocused())
        {
            if (e.key.code == sf::Keyboard::Space) { anim.togglePlay(); return; }
            if (e.key.code == sf::Keyboard::Left) { anim.stepBack(); return; }
            if (e.key.code == sf::Keyboard::Right) { anim.stepForward(); return; }
            if (e.key.code == sf::Keyboard::Z) { doUndo(); return; }
            if (e.key.code == sf::Keyboard::F)
            {
                float minX, minY, maxX, maxY;
                anim.getBounds(minX, minY, maxX, maxY);
                camera.fitTo(minX, minY, maxX, maxY);
                return;
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (tabButtons[i].handleEvent(e))
        {
            setTab(i);
            return;
        }
    }
    if (treeTabActive())
    {
        int op = opPanel.handleEvent(e);
        if (op != -1)
        {
            runOperation(op);
            return;
        }
        if (sizeSlider.handleEvent(e))
        {
            return;
        }
    }
    int action = toolbar.handleEvent(e);
    if (action == Toolbar::ACT_PLAY) { anim.togglePlay(); return; }
    if (action == Toolbar::ACT_STEP_BACK) { anim.stepBack(); return; }
    if (action == Toolbar::ACT_STEP_FWD) { anim.stepForward(); return; }
    if (action == Toolbar::ACT_UNDO) { doUndo(); return; }
    if (action == Toolbar::ACT_REPLAY) { anim.seek(0); anim.play(); return; }
    if (action == Toolbar::ACT_RESET) { runOperation(OP_RANDOM); return; }
    if (action == Toolbar::ACT_HINT) { setStatus("Try every operation - this wing is judgement-free."); return; }
    if (action == Toolbar::ACT_SPEED) { anim.setSpeed(toolbar.getSpeed()); return; }
    if (action == Toolbar::ACT_SCRUB)
    {
        anim.seek((int)(toolbar.getScrub() * (float)anim.totalEvents() + 0.5f));
        return;
    }
    if (action == Toolbar::ACT_FIT)
    {
        float minX, minY, maxX, maxY;
        anim.getBounds(minX, minY, maxX, maxY);
        camera.fitTo(minX, minY, maxX, maxY);
        return;
    }
    if (logPanel.handleEvent(e))
    {
        return;
    }

    // canvas: zoom, pan, select
    if (e.type == sf::Event::MouseWheelScrolled
        && camera.viewportContains((float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y))
    {
        camera.zoomAt(e.mouseWheelScroll.delta,
                      (float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
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
        if (!realDrag && camera.viewportContains(px, py) && treeTabActive())
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            int id = anim.findIdAt(world.x, world.y);
            selectedId = id;
            if (id != -1)
            {
                TreeNode* node = activeTree().findById(id);
                if (node != nullptr)
                {
                    setStatus("Selected room " + TextUtil::intToText(node->key));
                }
            }
        }
    }
    if (e.type == sf::Event::MouseMoved)
    {
        camera.dragTo((float)e.mouseMove.x, (float)e.mouseMove.y);
    }
}

void SandboxScreen::update(float dt)
{
    layoutWidgets();
    camera.update(dt);
    anim.update(dt);
    renderer.update(dt);
    for (int i = 0; i < 4; i++)
    {
        tabButtons[i].update(dt, mouseX, mouseY);
    }
    opPanel.update(dt, mouseX, mouseY);
    toolbar.update(dt, mouseX, mouseY, anim.isPlaying(), anim.progress01());

    if (treeTabActive() && camera.viewportContains(mouseX, mouseY))
    {
        sf::Vector2f world = camera.toWorld(app.window, mouseX, mouseY);
        hoveredId = anim.findIdAt(world.x, world.y);
    }
    else
    {
        hoveredId = -1;
    }
    if (hoveredId != -1)
    {
        NodeVisual* v = anim.findById(hoveredId);
        if (v != nullptr)
        {
            tooltip.show("Key " + TextUtil::intToText(v->key)
                         + "\nHeight " + TextUtil::intToText(v->heightVal)
                         + "   BF " + TextUtil::intToText(v->bf)
                         + "\nSize " + TextUtil::intToText(v->sizeVal)
                         + "   Depth " + TextUtil::intToText(v->depth), mouseX, mouseY);
        }
    }
    else
    {
        tooltip.hide();
    }
}

void SandboxScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);

    if (treeTabActive())
    {
        camera.apply(window);
        renderer.draw(window, anim, badgeMode, hoveredId, selectedId, -1);
        app.useUiView();
    }
    else
    {
        sf::Vector2u size = window.getSize();
        sf::FloatRect b = comingSoonText.getLocalBounds();
        comingSoonText.setPosition(((float)size.x - 380.f - b.width) / 2.f,
                                   (float)size.y / 2.f - 20.f);
        window.draw(comingSoonText);
    }

    titleText.setPosition(24.f, 10.f);
    window.draw(titleText);
    for (int i = 0; i < 4; i++)
    {
        tabButtons[i].draw(window);
    }

    if (treeTabActive())
    {
        opPanel.draw(window);
        sizeLabel.setString("Random size");
        sf::Vector2u size = window.getSize();
        sizeLabel.setPosition((float)size.x - 360.f - 16.f + 14.f, 492.f);
        window.draw(sizeLabel);
        sizeSlider.draw(window);
    }
    logPanel.draw(window, anim.getLog(), anim.currentIndex());
    toolbar.draw(window);

    statusText.setString(status);
    statusText.setPosition(24.f, 54.f);
    window.draw(statusText);
    tooltip.draw(window);
}

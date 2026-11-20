#include "screens/SandboxBScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "screens/SandboxScreen.h"
#include "model/HeapValidator.h"
#include "core/Random.h"
#include "core/TextUtil.h"

SandboxBScreen::SandboxBScreen(App& a) : app(a)
{
    tab = TAB_TREE;
    selectedId = -1;
    selectedIndex = -1;
    secondId = -1;
    hoveredId = -1;
    hoveredIndex = -1;
    seedCounter = 1;
    factLines = "";
    statusLine = "Pick an operation on the right.";
    mouseX = 0.f;
    mouseY = 0.f;
    rightX = 0.f;
    rightW = 0.f;
    arrayY = 0.f;
    ready = false;
}

void SandboxBScreen::onEnter()
{
    treeRenderer.init(&app.monoFont, &app.uiFont);
    heapView.init(&app.monoFont, &app.uiFont);
    arrayView.init(&app.monoFont, &app.uiFont);
    opPanel.init(&app.titleFont, &app.uiFont, "Operations");
    logPanel.init(&app.titleFont, &app.monoFont);
    tooltip.init(&app.uiFont);
    text.setFont(app.uiFont);
    text.setCharacterSize(Theme::smallSize());

    layout();
    buildOperationPanel();
    if (tree.getRoot() == nullptr)
    {
        newRandomTree();
    }
    if (heap.totalCount() == 0)
    {
        newRandomHeap();
    }
    showTreeStatic();
    ready = true;
}

void SandboxBScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    rightW = 360.f;
    rightX = w - rightW - 16.f;

    float canvasW = rightX - 8.f;
    arrayY = h - 96.f - 8.f;
    float canvasBottom = tab == TAB_HEAP ? arrayY - 8.f : h - 24.f;
    camera.setViewport(0.f, 72.f, canvasW, canvasBottom - 72.f, w, h);
    arrayView.setRect(8.f, arrayY, canvasW - 16.f, 96.f);

    string tabNames[3] = { "Binary Tree", "Max Heap", "BST / AVL (Partner A)" };
    float tabW = 180.f;
    for (int i = 0; i < 3; i++)
    {
        tabButtons[i].init(&app.uiFont, tabNames[i], 16.f + (float)i * (tabW + 8.f),
                           16.f, tabW, 42.f);
        tabButtons[i].setToggled(i == tab);
    }
    backButton.init(&app.uiFont, "Back", w - rightW - 16.f - 110.f, 16.f, 100.f, 42.f);

    opPanel.setRect(rightX, 16.f, rightW, 340.f);
    factPanel.setRect(rightX, 364.f, rightW, 150.f, Theme::panelRadius());
    factPanel.setFillColor(Theme::panel());
    factPanel.setOutline(Theme::panelBorder(), 1.f);
    logPanel.setRect(rightX, 522.f, rightW, h - 522.f - 24.f);
}

void SandboxBScreen::buildOperationPanel()
{
    opPanel.clearButtons();
    if (tab == TAB_TREE)
    {
        opPanel.addButton("New random tree");
        opPanel.addButton("Preorder (rec)");
        opPanel.addButton("Preorder (iter)");
        opPanel.addButton("Inorder (rec)");
        opPanel.addButton("Inorder (iter)");
        opPanel.addButton("Postorder (rec)");
        opPanel.addButton("Postorder (iter)");
        opPanel.addButton("Level order");
        opPanel.addButton("Mirror");
        opPanel.addButton("Height / diameter");
        opPanel.addButton("LCA of two rooms");
        opPanel.addButton("Path to room");
        opPanel.addButton("Is it symmetric?");
        opPanel.setFieldCount(1);
        opPanel.setFieldPlaceholders("rooms (1-20)", "");
    }
    else
    {
        opPanel.addButton("New random array");
        opPanel.addButton("Build Max Heap");
        opPanel.addButton("Insert value");
        opPanel.addButton("Extract Maximum");
        opPanel.addButton("Heapify");
        opPanel.addButton("Sift Down here");
        opPanel.addButton("Sift Up here");
        opPanel.addButton("Heap Sort");
        opPanel.addButton("Check heap property");
        opPanel.addButton("Reset counters");
        opPanel.setFieldCount(2);
        opPanel.setFieldPlaceholders("value", "size (1-24)");
    }
    opPanel.relayout();
}

void SandboxBScreen::setTab(int newTab)
{
    if (newTab == TAB_PARTNER_A)
    {
        app.push(new SandboxScreen(app));
        return;
    }
    tab = newTab;
    selectedId = -1;
    secondId = -1;
    selectedIndex = -1;
    buildOperationPanel();
    layout();
    if (tab == TAB_TREE)
    {
        showTreeStatic();
    }
    else
    {
        showHeapStatic();
    }
    statusLine = tab == TAB_TREE
        ? "Click rooms to select them, then try the traversals."
        : "Click a cell to select it, then sift it up or down.";
}

// ---- binary tree tab ----

void SandboxBScreen::newRandomTree()
{
    int wanted = 0;
    if (!opPanel.getFieldInt(0, wanted) || wanted < 1)
    {
        wanted = 9;
    }
    if (wanted > 20)
    {
        wanted = 20;
    }

    seedCounter++;
    Random rng(seedCounter * 7919 + 13);
    int values[24];
    for (int i = 0; i < wanted; i++)
    {
        values[i] = rng.nextInt(10, 99);
    }
    // distinct values, so a reconstruction from the traversals is unique
    for (int i = 0; i < wanted; i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (values[i] == values[j])
            {
                values[i] = values[i] % 99 + 1;
                j = -1;
            }
        }
    }

    // a random shape: pick a random root inside each inorder range
    int pre[24];
    int written = 0;
    int stackLo[52];
    int stackHi[52];
    int top = 0;
    stackLo[0] = 0;
    stackHi[0] = wanted - 1;
    top = 1;
    while (top > 0)
    {
        top--;
        int lo = stackLo[top];
        int hi = stackHi[top];
        if (lo > hi)
        {
            continue;
        }
        int split = rng.nextInt(lo, hi);
        pre[written] = values[split];
        written++;
        stackLo[top] = split + 1;
        stackHi[top] = hi;
        top++;
        stackLo[top] = lo;
        stackHi[top] = split - 1;
        top++;
    }

    tree.setEventLog(nullptr);
    tree.buildPreIn(pre, values, wanted);
    selectedId = -1;
    secondId = -1;
    statusLine = "Built a random vault of " + TextUtil::intToText(wanted) + " rooms.";
}

void SandboxBScreen::showTreeStatic()
{
    opLog.clear();
    opLog.saveTreeShape(tree.getRoot());
    treeAnim.startLog(&opLog, false);
    float minX, minY, maxX, maxY;
    treeAnim.getBounds(minX, minY, maxX, maxY);
    camera.fitTo(minX - 40.f, minY - 50.f, maxX + 40.f, maxY + 50.f);
    refreshFacts();
}

void SandboxBScreen::runTreeOperation(int button)
{
    int ids[Limits::MAX_NODES_GUI];

    if (button == T_RANDOM)
    {
        newRandomTree();
        showTreeStatic();
        return;
    }
    if (button == T_SHAPE_FACTS)
    {
        statusLine = "Height " + TextUtil::intToText(tree.height())
                   + ", diameter " + TextUtil::intToText(tree.diameter()) + " rooms.";
        refreshFacts();
        return;
    }
    if (button == T_SYMMETRY)
    {
        statusLine = tree.isSymmetric()
            ? "This vault IS its own mirror image."
            : "This vault is not symmetric.";
        return;
    }
    if (button == T_LCA)
    {
        if (selectedId == -1 || secondId == -1)
        {
            statusLine = "Select two rooms first (click one, then another).";
            return;
        }
        int answer = -1;
        if (tree.lca(selectedId, secondId, answer))
        {
            treeAnim.clearStates();
            treeAnim.setStateById(selectedId, AnimationController::STATE_TARGET);
            treeAnim.setStateById(secondId, AnimationController::STATE_TARGET);
            treeAnim.setStateById(answer, AnimationController::STATE_RESULT);
            NodeVisual* v = treeAnim.findById(answer);
            statusLine = "The two routes meet at room "
                       + TextUtil::intToText(v != nullptr ? v->key : 0) + ".";
        }
        else
        {
            statusLine = "Those two rooms have no common ancestor.";
        }
        return;
    }
    if (button == T_PATH)
    {
        if (selectedId == -1)
        {
            statusLine = "Select a room first.";
            return;
        }
        int length = tree.pathTo(selectedId, ids, Limits::MAX_NODES_GUI);
        treeAnim.clearStates();
        string keys = "";
        for (int i = 0; i < length; i++)
        {
            treeAnim.setStateById(ids[i], AnimationController::STATE_VISITED);
            NodeVisual* v = treeAnim.findById(ids[i]);
            if (i > 0)
            {
                keys = keys + " -> ";
            }
            keys = keys + TextUtil::intToText(v != nullptr ? v->key : 0);
        }
        statusLine = "Path (" + TextUtil::intToText(length) + " rooms): " + keys;
        return;
    }
    if (button == T_MIRROR)
    {
        opLog.clear();
        opLog.saveTreeShape(tree.getRoot());
        tree.setEventLog(&opLog);
        tree.mirror();
        tree.setEventLog(nullptr);
        treeAnim.setSpeed(app.save.animationSpeed);
        treeAnim.startLog(&opLog, true);
        statusLine = "Mirrored. Every left corridor is now a right one.";
        refreshFacts();
        return;
    }

    // the seven traversals all animate the same way
    opLog.clear();
    opLog.saveTreeShape(tree.getRoot());
    tree.setEventLog(&opLog);
    int count = 0;
    string name = "";
    if (button == T_PREORDER_REC) { count = tree.preorderRec(ids, Limits::MAX_NODES_GUI); name = "Preorder (recursive)"; }
    else if (button == T_PREORDER_ITER) { count = tree.preorderIter(ids, Limits::MAX_NODES_GUI); name = "Preorder (iterative)"; }
    else if (button == T_INORDER_REC) { count = tree.inorderRec(ids, Limits::MAX_NODES_GUI); name = "Inorder (recursive)"; }
    else if (button == T_INORDER_ITER) { count = tree.inorderIter(ids, Limits::MAX_NODES_GUI); name = "Inorder (iterative)"; }
    else if (button == T_POSTORDER_REC) { count = tree.postorderRec(ids, Limits::MAX_NODES_GUI); name = "Postorder (recursive)"; }
    else if (button == T_POSTORDER_ITER) { count = tree.postorderIter(ids, Limits::MAX_NODES_GUI); name = "Postorder (iterative)"; }
    else if (button == T_LEVEL_ORDER) { count = tree.levelOrder(ids, Limits::MAX_NODES_GUI); name = "Level order"; }
    tree.setEventLog(nullptr);

    treeAnim.setSpeed(app.save.animationSpeed);
    treeAnim.startLog(&opLog, true);

    string keys = "";
    for (int i = 0; i < count && i < 20; i++)
    {
        NodeVisual* v = treeAnim.findById(ids[i]);
        if (i > 0)
        {
            keys = keys + " ";
        }
        keys = keys + TextUtil::intToText(v != nullptr ? v->key : 0);
    }
    statusLine = name + ": " + keys;
}

// ---- max heap tab ----

void SandboxBScreen::newRandomHeap()
{
    int wanted = 0;
    if (!opPanel.getFieldInt(1, wanted) || wanted < 1)
    {
        wanted = 10;
    }
    if (wanted > 24)
    {
        wanted = 24;
    }
    seedCounter++;
    Random rng(seedCounter * 104729 + 7);
    int values[24];
    for (int i = 0; i < wanted; i++)
    {
        values[i] = rng.nextInt(10, 99);
    }
    heap.setEventLog(nullptr);
    heap.load(values, wanted, wanted);
    heap.resetCounters();
    selectedIndex = -1;
    statusLine = "Loaded " + TextUtil::intToText(wanted)
               + " unsorted treasures. Build Max Heap to organise them.";
}

void SandboxBScreen::showHeapStatic()
{
    int shown[Limits::MAX_NODES_GUI];
    int n = heap.totalCount();
    for (int i = 0; i < n; i++)
    {
        shown[i] = heap.get(i);
    }
    opLog.clear();
    heapAnim.showStatic(shown, n, heap.getHeapSize());
    float minX, minY, maxX, maxY;
    heapView.getBounds(heapAnim, minX, minY, maxX, maxY);
    camera.fitTo(minX - 40.f, minY - 50.f, maxX + 40.f, maxY + 50.f);
    refreshFacts();
}

void SandboxBScreen::beginHeapOperation()
{
    int before[Limits::MAX_NODES_GUI];
    int n = heap.totalCount();
    for (int i = 0; i < n; i++)
    {
        before[i] = heap.get(i);
    }
    opLog.clear();
    opLog.saveArray(before, n);
    heap.setEventLog(&opLog);
}

void SandboxBScreen::endHeapOperation()
{
    heap.setEventLog(nullptr);
    if (opLog.size() > 0 && opLog.get(opLog.size() - 1).type != Event::DONE)
    {
        Event e;
        e.type = Event::DONE;
        e.text = "Operation complete";
        opLog.add(e);
    }
    heapAnim.setSpeed(app.save.animationSpeed);
    heapAnim.start(&opLog, true, heapAnim.getHeapSize());
    refreshFacts();
}

void SandboxBScreen::runHeapOperation(int button)
{
    if (button == H_RANDOM)
    {
        newRandomHeap();
        showHeapStatic();
        return;
    }
    if (button == H_RESET)
    {
        heap.resetCounters();
        statusLine = "Comparison and swap counters cleared.";
        refreshFacts();
        return;
    }
    if (button == H_CHECK)
    {
        string error;
        statusLine = HeapValidator::checkHeap(heap, error)
            ? "Valid: every parent is at least as large as both children."
            : "Not a Max Heap: " + error;
        return;
    }
    if (button == H_HEAP_SORT)
    {
        int input[Limits::MAX_NODES_GUI];
        int sorted[Limits::MAX_NODES_GUI];
        int n = heap.totalCount();
        for (int i = 0; i < n; i++)
        {
            input[i] = heap.get(i);
        }
        long long beforeComparisons = heap.getComparisons();
        heap.heapSort(input, n, sorted);
        heap.load(sorted, n, 0);          // everything is sorted, nothing active
        showHeapStatic();
        statusLine = "Heap Sort finished in "
                   + TextUtil::intToText((int)(heap.getComparisons() - beforeComparisons))
                   + " comparisons. The whole array is the sorted region now.";
        return;
    }
    if (button == H_INSERT)
    {
        int value = 0;
        if (!opPanel.getFieldInt(0, value))
        {
            statusLine = "Type a value in the first box, then press Insert.";
            return;
        }
        beginHeapOperation();
        heap.insert(value);
        endHeapOperation();
        statusLine = "Inserted " + TextUtil::intToText(value) + " and sifted it up.";
        return;
    }
    if (button == H_SIFT_DOWN || button == H_SIFT_UP)
    {
        if (selectedIndex == -1)
        {
            statusLine = "Select a cell first, then sift it.";
            return;
        }
        int target = selectedIndex;
        beginHeapOperation();
        if (button == H_SIFT_DOWN)
        {
            heap.siftDown(target, heap.getHeapSize());
        }
        else
        {
            heap.siftUp(target);
        }
        endHeapOperation();
        statusLine = (button == H_SIFT_DOWN ? "Sifted index " : "Lifted index ")
                   + TextUtil::intToText(target) + ".";
        return;
    }

    beginHeapOperation();
    if (button == H_BUILD)
    {
        int values[Limits::MAX_NODES_GUI];
        int n = heap.totalCount();
        for (int i = 0; i < n; i++)
        {
            values[i] = heap.get(i);
        }
        heap.buildMaxHeap(values, n);
        statusLine = "Built bottom-up from the last parent to the root.";
    }
    else if (button == H_HEAPIFY)
    {
        heap.heapify();
        statusLine = "Heapified the active region.";
    }
    else if (button == H_EXTRACT)
    {
        int taken = 0;
        if (heap.extractMax(taken))
        {
            statusLine = "Extracted " + TextUtil::intToText(taken)
                       + " into the sorted region.";
        }
        else
        {
            statusLine = "The active heap is empty.";
        }
    }
    endHeapOperation();
}

void SandboxBScreen::refreshFacts()
{
    if (tab == TAB_TREE)
    {
        int ids[Limits::MAX_NODES_GUI];
        tree.setEventLog(nullptr);
        int n = tree.preorderRec(ids, Limits::MAX_NODES_GUI);
        factLines = "Rooms      " + TextUtil::intToText(n) + "\n"
                  + "Height     " + TextUtil::intToText(tree.height()) + "\n"
                  + "Diameter   " + TextUtil::intToText(tree.diameter()) + "\n"
                  + "Symmetric  " + (tree.isSymmetric() ? "yes" : "no");
    }
    else
    {
        string error;
        bool valid = HeapValidator::checkHeap(heap, error);
        factLines = "Stored       " + TextUtil::intToText(heap.totalCount()) + "\n"
                  + "Active heap  " + TextUtil::intToText(heap.getHeapSize()) + "\n"
                  + "Comparisons  " + TextUtil::intToText((int)heap.getComparisons()) + "\n"
                  + "Swaps        " + TextUtil::intToText((int)heap.getSwaps()) + "\n"
                  + "Max Heap     " + (valid ? "yes" : "no");
    }
}

// ---- screen plumbing ----

void SandboxBScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        mouseX = (float)e.mouseMove.x;
        mouseY = (float)e.mouseMove.y;
    }
    if (e.type == sf::Event::Resized)
    {
        layout();
    }
    if (e.type == sf::Event::KeyPressed && !opPanel.anyFieldFocused()
        && e.key.code == sf::Keyboard::Escape)
    {
        app.requestPop();
        return;
    }
    if (backButton.handleEvent(e))
    {
        app.requestPop();
        return;
    }
    for (int i = 0; i < 3; i++)
    {
        if (tabButtons[i].handleEvent(e))
        {
            setTab(i);
            return;
        }
    }
    int button = opPanel.handleEvent(e);
    if (button >= 0)
    {
        if (tab == TAB_TREE)
        {
            runTreeOperation(button);
        }
        else
        {
            runHeapOperation(button);
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
        if (tab == TAB_HEAP)
        {
            int cell = arrayView.indexAt(heapAnim, px, py);
            if (cell != -1)
            {
                selectedIndex = cell;
                return;
            }
        }
        if (camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            int hit = tab == TAB_TREE ? treeAnim.findIdAt(world.x, world.y)
                                      : heapView.indexAt(heapAnim, world.x, world.y);
            if (hit == -1)
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
        if (!realDrag && camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            if (tab == TAB_TREE)
            {
                int id = treeAnim.findIdAt(world.x, world.y);
                if (id != -1)
                {
                    // the two most recent clicks are what LCA works on
                    secondId = selectedId;
                    selectedId = id;
                    treeAnim.clearStates();
                    treeAnim.setStateById(selectedId, AnimationController::STATE_SELECTED);
                    if (secondId != -1)
                    {
                        treeAnim.setStateById(secondId, AnimationController::STATE_TARGET);
                    }
                }
            }
            else
            {
                int index = heapView.indexAt(heapAnim, world.x, world.y);
                if (index != -1)
                {
                    selectedIndex = index;
                }
            }
        }
    }
    if (e.type == sf::Event::MouseMoved)
    {
        camera.dragTo((float)e.mouseMove.x, (float)e.mouseMove.y);
    }
}

void SandboxBScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    layout();
    camera.update(dt);
    treeRenderer.update(dt);
    heapView.update(dt);
    treeAnim.update(dt);
    heapAnim.update(dt);
    opPanel.update(dt, mouseX, mouseY);
    backButton.update(dt, mouseX, mouseY);
    for (int i = 0; i < 3; i++)
    {
        tabButtons[i].update(dt, mouseX, mouseY);
    }

    hoveredId = -1;
    hoveredIndex = -1;
    if (camera.viewportContains(mouseX, mouseY))
    {
        sf::Vector2f world = camera.toWorld(app.window, mouseX, mouseY);
        if (tab == TAB_TREE)
        {
            hoveredId = treeAnim.findIdAt(world.x, world.y);
        }
        else
        {
            hoveredIndex = heapView.indexAt(heapAnim, world.x, world.y);
        }
    }
    if (tab == TAB_HEAP && hoveredIndex == -1)
    {
        hoveredIndex = arrayView.indexAt(heapAnim, mouseX, mouseY);
    }

    if (app.save.showTooltips && tab == TAB_TREE && hoveredId != -1)
    {
        NodeVisual* v = treeAnim.findById(hoveredId);
        if (v != nullptr)
        {
            tooltip.show("Value " + TextUtil::intToText(v->key)
                         + "\nHeight " + TextUtil::intToText(v->heightVal)
                         + "\nSubtree rooms " + TextUtil::intToText(v->sizeVal)
                         + "\nDepth " + TextUtil::intToText(v->depth), mouseX, mouseY);
        }
    }
    else if (app.save.showTooltips && tab == TAB_HEAP && hoveredIndex != -1)
    {
        tooltip.show("Value " + TextUtil::intToText(heapAnim.getValue(hoveredIndex))
                     + "\nIndex " + TextUtil::intToText(hoveredIndex)
                     + "\nChildren " + TextUtil::intToText(2 * hoveredIndex + 1)
                     + ", " + TextUtil::intToText(2 * hoveredIndex + 2), mouseX, mouseY);
    }
    else
    {
        tooltip.hide();
    }
}

void SandboxBScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);

    camera.apply(window);
    if (tab == TAB_TREE)
    {
        treeRenderer.draw(window, treeAnim, TreeRenderer::BADGE_HEIGHT,
                          hoveredId, selectedId, secondId);
    }
    else
    {
        heapView.draw(window, heapAnim, selectedIndex, hoveredIndex);
    }

    app.useUiView();
    if (tab == TAB_HEAP)
    {
        arrayView.draw(window, heapAnim, selectedIndex, hoveredIndex);
    }
    for (int i = 0; i < 3; i++)
    {
        tabButtons[i].draw(window);
    }
    backButton.draw(window);
    opPanel.draw(window);
    logPanel.draw(window, tab == TAB_TREE ? treeAnim.getLog() : heapAnim.getLog(),
                  tab == TAB_TREE ? treeAnim.currentIndex() : heapAnim.currentIndex());

    factPanel.draw(window);
    text.setFillColor(Theme::text());
    text.setString(factLines);
    text.setPosition(factPanel.left() + 18.f, factPanel.top() + 16.f);
    window.draw(text);

    text.setFillColor(Theme::gold());
    text.setString(statusLine);
    text.setPosition(16.f, (float)window.getSize().y - 20.f);
    window.draw(text);

    tooltip.draw(window);
}

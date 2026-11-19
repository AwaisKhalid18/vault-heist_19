#include "screens/EndlessScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "model/HeapValidator.h"
#include "core/TextUtil.h"

EndlessScreen::EndlessScreen(App& a) : app(a), rng(1)
{
    state = ST_READY;
    survivedSeconds = 0.f;
    nextDeliveryIn = 0.f;
    deliveryInterval = 2.2f;
    delivered = 0;
    extracted = 0;
    score = 0;
    mouseX = 0.f;
    mouseY = 0.f;
    rightX = 0.f;
    rightW = 0.f;
    ready = false;
}

void EndlessScreen::onEnter()
{
    heapView.init(&app.monoFont, &app.uiFont);
    arrayView.init(&app.monoFont, &app.uiFont);
    hud.init(&app.titleFont, &app.uiFont);
    opPanel.init(&app.titleFont, &app.uiFont, "Survival");
    dialog.init(&app.titleFont, &app.uiFont);
    line.setFont(app.uiFont);
    line.setCharacterSize(Theme::smallSize());

    opPanel.clearButtons();
    opPanel.addButton("Extract Maximum");
    opPanel.addButton("Heapify");
    opPanel.addButton("Start run");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    layout();

    if (state == ST_READY)
    {
        heap.setEventLog(nullptr);
        heap.load(nullptr, 0, 0);
        showHeap();
        hud.setObjective("Keep the vault under " + TextUtil::intToText(CAPACITY)
                         + " treasures. Extract the maximum to make room.");
    }
    ready = true;
}

void EndlessScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    rightW = 360.f;
    rightX = w - rightW - 16.f;

    float canvasW = rightX - 8.f;
    float arrayH = 96.f;
    float arrayY = h - arrayH - 24.f;
    camera.setViewport(0.f, 0.f, canvasW, arrayY - 8.f, w, h);
    arrayView.setRect(8.f, arrayY, canvasW - 16.f, arrayH);

    hud.setRect(rightX, 16.f, rightW, 196.f);
    infoPanel.setRect(rightX, 224.f, rightW, 190.f, Theme::panelRadius());
    infoPanel.setFillColor(Theme::panel());
    infoPanel.setOutline(Theme::panelBorder(), 1.f);
    opPanel.setRect(rightX, 428.f, rightW, 206.f);
}

void EndlessScreen::showHeap()
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

    float minX, minY, maxX, maxY;
    heapView.getBounds(anim, minX, minY, maxX, maxY);
    camera.fitTo(minX - 40.f, minY - 50.f, maxX + 40.f, maxY + 50.f);
}

void EndlessScreen::startRun()
{
    rng.reseed(app.save.endlessSeed);
    heap.setEventLog(nullptr);
    heap.load(nullptr, 0, 0);
    // a few treasures to begin with, so the vault is never empty
    for (int i = 0; i < 4; i++)
    {
        heap.insert(rng.nextInt(10, 999));
    }
    survivedSeconds = 0.f;
    deliveryInterval = 2.2f;
    nextDeliveryIn = deliveryInterval;
    delivered = 4;
    extracted = 0;
    score = 0;
    state = ST_RUNNING;
    showHeap();
}

int EndlessScreen::liveScore()
{
    // survival time is the score, with a small bonus per treasure handled
    return (int)(survivedSeconds * 10.f) + extracted * 5;
}

void EndlessScreen::deliverTreasure()
{
    if (heap.getHeapSize() >= CAPACITY)
    {
        endRun("The vault overflowed. The guards walked in on a jammed mechanism.");
        return;
    }
    int before = heap.totalCount();
    int values[Limits::MAX_NODES_GUI];
    for (int i = 0; i < before; i++)
    {
        values[i] = heap.get(i);
    }
    opLog.clear();
    opLog.saveArray(values, before);
    heap.setEventLog(&opLog);
    heap.insert(rng.nextInt(10, 999));
    heap.setEventLog(nullptr);
    delivered++;
    anim.setSpeed(2.f);
    anim.start(&opLog, true, before);

    string error;
    if (!HeapValidator::checkHeap(heap, error))
    {
        endRun("The mechanism lost its heap property: " + error);
    }
}

void EndlessScreen::extractOnce()
{
    if (state != ST_RUNNING || heap.getHeapSize() <= 0)
    {
        return;
    }
    int before = heap.totalCount();
    int values[Limits::MAX_NODES_GUI];
    for (int i = 0; i < before; i++)
    {
        values[i] = heap.get(i);
    }
    int startHeapSize = heap.getHeapSize();
    opLog.clear();
    opLog.saveArray(values, before);
    heap.setEventLog(&opLog);
    int taken = 0;
    heap.extractMax(taken);
    heap.setEventLog(nullptr);

    // the extracted treasure leaves the vault completely, so the array keeps
    // only the active heap and the run can go on for ever
    int live[Limits::MAX_NODES_GUI];
    int liveCount = heap.getHeapSize();
    for (int i = 0; i < liveCount; i++)
    {
        live[i] = heap.get(i);
    }
    extracted++;
    anim.setSpeed(2.f);
    anim.start(&opLog, true, startHeapSize);
    heap.load(live, liveCount, liveCount);

    string error;
    if (!HeapValidator::checkHeap(heap, error))
    {
        endRun("The mechanism lost its heap property: " + error);
    }
}

void EndlessScreen::endRun(const string& reason)
{
    if (state == ST_OVER)
    {
        return;
    }
    state = ST_OVER;
    score = liveScore();
    showHeap();
    // the run goes straight onto the local leaderboard
    app.save.addScore("Thief " + TextUtil::intToText(app.save.scoreCount() + 1), score);
    app.save.save(app.savePath);
    dialog.open2("Run over",
                 reason + "\n\nSurvived " + TextUtil::intToText((int)survivedSeconds)
                 + " seconds\nTreasures delivered " + TextUtil::intToText(delivered)
                 + "\nMaxima extracted " + TextUtil::intToText(extracted)
                 + "\n\nScore " + TextUtil::intToText(score)
                 + "\n\nThe run has been saved to the leaderboard.",
                 "Close", "");
}

void EndlessScreen::handleEvent(const sf::Event& e)
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
    if (dialog.isOpen())
    {
        if (dialog.handleEvent(e) >= 0)
        {
            dialog.close();
        }
        return;
    }
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
    {
        app.requestPop();
        return;
    }

    int button = opPanel.handleEvent(e);
    if (button == OP_START)
    {
        startRun();
        return;
    }
    if (button == OP_EXTRACT)
    {
        extractOnce();
        return;
    }
    if (button == OP_HEAPIFY && state == ST_RUNNING)
    {
        heap.setEventLog(nullptr);
        heap.heapify();
        showHeap();
        return;
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float px = (float)e.mouseButton.x;
        float py = (float)e.mouseButton.y;
        // clicking the root is the fast way to extract
        if (camera.viewportContains(px, py))
        {
            sf::Vector2f world = camera.toWorld(app.window, px, py);
            if (heapView.indexAt(anim, world.x, world.y) == 0)
            {
                extractOnce();
            }
        }
        else if (arrayView.indexAt(anim, px, py) == 0)
        {
            extractOnce();
        }
    }
}

void EndlessScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    layout();
    camera.update(dt);
    anim.update(dt);
    heapView.update(dt);
    dialog.update(dt, mouseX, mouseY);
    opPanel.update(dt, mouseX, mouseY);

    if (state == ST_RUNNING)
    {
        survivedSeconds += dt;
        nextDeliveryIn -= dt;
        if (nextDeliveryIn <= 0.f)
        {
            deliverTreasure();
            // the vault speeds up the longer the run lasts
            deliveryInterval = 2.2f - survivedSeconds * 0.02f;
            if (deliveryInterval < 0.45f)
            {
                deliveryInterval = 0.45f;
            }
            nextDeliveryIn = deliveryInterval;
        }
    }

    hud.setAlarm(heap.getHeapSize(), CAPACITY);
    hud.setScoreInfo(liveScore(), (int)survivedSeconds, delivered);
    hud.update(dt);
}

void EndlessScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);

    camera.apply(window);
    heapView.draw(window, anim, -1, -1);

    app.useUiView();
    arrayView.draw(window, anim, -1, -1);
    hud.draw(window);
    infoPanel.draw(window);
    opPanel.draw(window);

    float x = infoPanel.left() + 20.f;
    float y = infoPanel.top() + 18.f;
    line.setFillColor(Theme::gold());
    line.setString(state == ST_RUNNING ? "RUN IN PROGRESS"
                   : (state == ST_OVER ? "RUN OVER" : "PRESS START RUN"));
    line.setPosition(x, y);
    window.draw(line);
    y += 28.f;

    line.setFillColor(Theme::text());
    line.setString("Survived      " + TextUtil::intToText((int)survivedSeconds) + " s");
    line.setPosition(x, y);
    window.draw(line);
    y += 22.f;
    line.setString("Delivered     " + TextUtil::intToText(delivered));
    line.setPosition(x, y);
    window.draw(line);
    y += 22.f;
    line.setString("Extracted     " + TextUtil::intToText(extracted));
    line.setPosition(x, y);
    window.draw(line);
    y += 22.f;
    line.setString("Vault         " + TextUtil::intToText(heap.getHeapSize()) + " / "
                   + TextUtil::intToText(CAPACITY));
    line.setFillColor(heap.getHeapSize() > CAPACITY - 5 ? Theme::violation() : Theme::text());
    line.setPosition(x, y);
    window.draw(line);
    y += 22.f;
    line.setFillColor(Theme::dimText());
    line.setString("Next delivery in "
                   + TextUtil::intToText((int)(nextDeliveryIn * 10.f)) + " tenths");
    line.setPosition(x, y);
    window.draw(line);

    line.setFillColor(Theme::dimText());
    line.setString("Click the gold root, or press Extract Maximum, to make room."
                   "   |   Esc leaves the run.");
    line.setPosition(16.f, (float)window.getSize().y - 18.f);
    window.draw(line);

    dialog.draw(window);
}

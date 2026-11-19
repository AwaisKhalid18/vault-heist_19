#include "screens/Level41Build.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"

Level41Build::Level41Build(App& a, const LevelDef& d) : HeapLevelBase(a, d)
{
    startCount = 0;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        startValues[i] = 0;
    }
}

void Level41Build::onLevelStart()
{
    startCount = LevelBuilderB::buildArray(def, startValues, Limits::MAX_NODES_GUI);
    loadHeap(startValues, startCount, startCount);

    // the optimal step count is what bottom-up construction needs on this
    // exact array, computed by the model rather than written into the JSON
    optimalSteps = LevelBuilderB::optimalBuildSwaps(startValues, startCount);
    if (optimalSteps < 1)
    {
        optimalSteps = 1;
    }

    opPanel.clearButtons();
    opPanel.addButton("Sift Down here");
    opPanel.addButton("Sift Up here");
    opPanel.addButton("Heapify all");
    opPanel.addButton("Clear selection");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Swap a parent with one of its children until every parent "
                 "holds at least as much as both children.");
    showHeapStatic(true);
}

void Level41Build::onCellClicked(int index)
{
    selectOrSwap(index);
}

void Level41Build::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_CLEAR)
    {
        selectedIndex = -1;
        return;
    }
    if (buttonIndex != OP_SIFT_DOWN && buttonIndex != OP_SIFT_UP && buttonIndex != OP_HEAPIFY)
    {
        return;
    }
    if (buttonIndex != OP_HEAPIFY && selectedIndex == -1)
    {
        flash("Select a room first, then sift it.");
        return;
    }

    // These buttons are shortcuts for the chain of swaps the operation would
    // perform by hand, so each is charged exactly the alarm those swaps cost.
    int before = (int)heap.getSwaps();
    int target = selectedIndex;
    selectedIndex = -1;
    saveUndo();
    beginOperation();
    if (buttonIndex == OP_SIFT_DOWN)
    {
        heap.siftDown(target, heap.getHeapSize());
    }
    else if (buttonIndex == OP_SIFT_UP)
    {
        heap.siftUp(target);
    }
    else
    {
        heap.heapify();
    }
    endOperation(true);

    int used = (int)heap.getSwaps() - before;
    if (used <= 0)
    {
        flash("Nothing moved - that part of the vault was already in order.");
        addAlarm(1);
    }
    else
    {
        addAlarm(used);
    }
}

void Level41Build::onUndoRestored()
{
    selectedIndex = -1;
}

void Level41Build::onAnimationDone()
{
    checkForWin();
}

void Level41Build::checkForWin()
{
    if (heap.isMaxHeap() && heap.getHeapSize() == startCount)
    {
        win("Every parent now outweighs its children. The Heap Fortress is awake.");
    }
}

void Level41Build::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        hintText.setFont(app.uiFont);
        hintText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    string line = selectedIndex == -1
        ? "Click a room, then click its parent or one of its children to swap them."
        : "Index " + TextUtil::intToText(selectedIndex)
          + " selected - now click its parent or one of its children.";
    hintText.setString(line);
    hintText.setFillColor(selectedIndex == -1 ? Theme::dimText() : Theme::currentVisit());
    hintText.setPosition(16.f, 16.f);
    window.draw(hintText);
}

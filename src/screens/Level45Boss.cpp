#include "screens/Level45Boss.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"

Level45Boss::Level45Boss(App& a, const LevelDef& d) : HeapLevelBase(a, d)
{
    startCount = 0;
    phase = PHASE_BUILD;
    buildOptimal = 1;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        startValues[i] = 0;
    }
}

void Level45Boss::onLevelStart()
{
    startCount = LevelBuilderB::buildArray(def, startValues, Limits::MAX_NODES_GUI);
    loadHeap(startValues, startCount, startCount);
    phase = PHASE_BUILD;

    // the whole workflow: the swaps bottom-up construction needs, then one
    // extraction per treasure. Both halves are measured, not guessed.
    buildOptimal = LevelBuilderB::optimalBuildSwaps(startValues, startCount);
    if (buildOptimal < 1)
    {
        buildOptimal = 1;
    }
    optimalSteps = buildOptimal + startCount;

    opPanel.clearButtons();
    opPanel.addButton("Build Max Heap");
    opPanel.addButton("Extract Maximum");
    opPanel.addButton("Heap Sort (auto)");
    opPanel.addButton("Clear selection");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Phase 1 of 2: build a valid Max Heap by swapping a parent "
                 "with one of its children.");
    showHeapStatic(true);
}

void Level45Boss::onCellClicked(int index)
{
    if (phase == PHASE_BUILD)
    {
        selectOrSwap(index);
        return;
    }
    if (phase == PHASE_SORT)
    {
        if (index != 0)
        {
            penalty(1, 0, "Take the root: a Max Heap keeps its maximum at index 0.");
            return;
        }
        extractOnce();
    }
}

void Level45Boss::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_CLEAR)
    {
        selectedIndex = -1;
        return;
    }

    if (buttonIndex == OP_BUILD)
    {
        if (phase != PHASE_BUILD)
        {
            flash("The heap is already built - you are sorting now.");
            return;
        }
        // the shortcut costs exactly the swaps it performs
        int before = (int)heap.getSwaps();
        selectedIndex = -1;
        saveUndo();
        beginOperation();
        heap.heapify();
        endOperation(true);
        int used = (int)heap.getSwaps() - before;
        addAlarm(used > 0 ? used : 1);
        return;
    }

    if (buttonIndex == OP_EXTRACT)
    {
        if (phase != PHASE_SORT)
        {
            flash("Build a valid Max Heap first - the root is not the maximum yet.");
            return;
        }
        extractOnce();
        return;
    }

    if (buttonIndex == OP_HEAP_SORT)
    {
        if (phase == PHASE_BUILD)
        {
            flash("Build a valid Max Heap first, then Heap Sort can finish the job.");
            return;
        }
        // run the remaining extractions in one batch, charged one alarm each
        int remaining = heap.getHeapSize();
        if (remaining <= 0)
        {
            return;
        }
        selectedIndex = -1;
        saveUndo();
        beginOperation();
        for (int i = 0; i < remaining; i++)
        {
            int taken = 0;
            heap.extractMax(taken);
        }
        endOperation(true);
        addAlarm(remaining);
    }
}

void Level45Boss::onUndoRestored()
{
    selectedIndex = -1;
    // the phase follows the heap: it is the build phase until the heap is valid
    if (phase != PHASE_DONE)
    {
        phase = heap.isMaxHeap() && heap.getHeapSize() == startCount ? PHASE_SORT : PHASE_BUILD;
    }
}

void Level45Boss::enterSortPhase()
{
    phase = PHASE_SORT;
    setObjective("Phase 2 of 2: take the root again and again until the "
                 "active heap is empty and the array reads in ascending order.");
    flash("Max Heap built. Now empty it, largest first.");
}

bool Level45Boss::arrayIsAscending()
{
    for (int i = 1; i < heap.totalCount(); i++)
    {
        if (heap.get(i) < heap.get(i - 1))
        {
            return false;
        }
    }
    return true;
}

void Level45Boss::onAnimationDone()
{
    checkForWin();
}

void Level45Boss::checkForWin()
{
    if (phase == PHASE_BUILD)
    {
        if (heap.isMaxHeap() && heap.getHeapSize() == startCount)
        {
            enterSortPhase();
        }
        return;
    }
    if (phase != PHASE_SORT)
    {
        return;
    }
    // the final door opens only on a fully sorted array
    if (heap.getHeapSize() == 0 && arrayIsAscending())
    {
        phase = PHASE_DONE;
        win("Final Vault Unlocked. Heap Sort left every treasure in ascending order.");
    }
}

void Level45Boss::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        statusText.setFont(app.uiFont);
        statusText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    string line;
    if (phase == PHASE_BUILD)
    {
        line = "PHASE 1 - BUILD   |   ";
        line = line + (selectedIndex == -1
            ? "click a room, then its parent or one of its children"
            : "index " + TextUtil::intToText(selectedIndex) + " selected");
    }
    else
    {
        line = "PHASE 2 - SORT   |   active heap "
             + TextUtil::intToText(heap.getHeapSize()) + " rooms, sorted "
             + TextUtil::intToText(heap.totalCount() - heap.getHeapSize())
             + "   |   click the gold root";
    }
    statusText.setString(line);
    statusText.setFillColor(phase == PHASE_BUILD ? Theme::warning() : Theme::balanced());
    statusText.setPosition(16.f, 16.f);
    window.draw(statusText);
}

#include "screens/Level43Extract.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelBuilderB.h"
#include "core/TextUtil.h"

Level43Extract::Level43Extract(App& a, const LevelDef& d) : HeapLevelBase(a, d)
{
    startCount = 0;
    targetExtractions = 0;
    extracted = 0;
    fontReady = false;
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        startValues[i] = 0;
    }
}

void Level43Extract::onLevelStart()
{
    startCount = LevelBuilderB::buildArray(def, startValues, Limits::MAX_NODES_GUI);
    loadHeap(startValues, startCount, startCount);

    // The vault must already be a valid Max Heap before extraction begins,
    // whatever shape the generator produced. loadHeap left the event log
    // detached, so this costs the player nothing and is not animated.
    heap.heapify();

    targetExtractions = def.k > 0 && def.k < startCount ? def.k : startCount;
    extracted = 0;
    // one extraction per treasure asked for: the optimum is the number of
    // extractions, computed from the level data, never typed in
    optimalSteps = targetExtractions;

    opPanel.clearButtons();
    opPanel.addButton("Extract Maximum");
    opPanel.addButton("Heapify");
    opPanel.setFieldCount(0);
    opPanel.relayout();

    setObjective("Take the " + TextUtil::intToText(targetExtractions)
                 + " largest treasures. The maximum is always the root, index 0.");
    showHeapStatic(true);
}

void Level43Extract::onCellClicked(int index)
{
    if (index != 0)
    {
        penalty(1, 0, "The maximum of a Max Heap is always at index 0. Take the root.");
        return;
    }
    doExtraction();
}

void Level43Extract::onOperation(int buttonIndex)
{
    if (buttonIndex == OP_EXTRACT)
    {
        doExtraction();
        return;
    }
    if (buttonIndex == OP_HEAPIFY)
    {
        if (heap.isMaxHeap())
        {
            flash("The active heap already satisfies the Max Heap property.");
            return;
        }
        saveUndo();
        beginOperation();
        heap.heapify();
        endOperation(true);
        addAlarm(1);
    }
}

void Level43Extract::doExtraction()
{
    if (extracted >= targetExtractions || heap.getHeapSize() <= 0)
    {
        flash("The contract is already filled.");
        return;
    }
    if (extractOnce())
    {
        extracted++;
    }
}

void Level43Extract::onUndoRestored()
{
    if (extracted > 0)
    {
        extracted--;
    }
}

void Level43Extract::onAnimationDone()
{
    if (extracted < targetExtractions)
    {
        return;
    }
    if (!heapIsValid())
    {
        return;
    }
    win("Every extracted maximum landed in the sorted region in order.");
}

void Level43Extract::drawUiOverlay(sf::RenderWindow& window)
{
    if (!fontReady)
    {
        statusText.setFont(app.uiFont);
        statusText.setCharacterSize(Theme::smallSize());
        fontReady = true;
    }
    statusText.setString("Extracted " + TextUtil::intToText(extracted) + " of "
                         + TextUtil::intToText(targetExtractions)
                         + "   |   active heap " + TextUtil::intToText(heap.getHeapSize())
                         + " rooms   |   click the gold root to take the maximum");
    statusText.setFillColor(Theme::dimText());
    statusText.setPosition(16.f, 16.f);
    window.draw(statusText);
}

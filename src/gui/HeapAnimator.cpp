#include "gui/HeapAnimator.h"
#include "gui/Theme.h"

HeapAnimator::HeapAnimator()
{
    log = nullptr;
    count = 0;
    heapSize = 0;
    initialHeapSize = 0;
    sortComplete = false;
    highlightA = -1;
    highlightB = -1;
    activeIndex = -1;
    eventIndex = 0;
    eventTime = 0.f;
    eventStarted = false;
    playing = false;
    pauseAfterEvent = false;
    speed = 1.f;
    swapping = false;
    swapT = 0.f;
    pendingSnapshot = -1;
    for (int i = 0; i < Limits::MAX_NODES_GUI; i++)
    {
        values[i] = 0;
    }
}

void HeapAnimator::loadSnapshot(int snapshotIndex)
{
    if (log == nullptr || snapshotIndex < 0)
    {
        return;
    }
    int n = log->getArray(snapshotIndex, values);
    count = n;
}

void HeapAnimator::showStatic(const int shown[], int n, int activeHeapSize)
{
    if (n > Limits::MAX_NODES_GUI)
    {
        n = Limits::MAX_NODES_GUI;
    }
    for (int i = 0; i < n; i++)
    {
        values[i] = shown[i];
    }
    count = n;
    heapSize = activeHeapSize;
    initialHeapSize = activeHeapSize;
    highlightA = -1;
    highlightB = -1;
    activeIndex = -1;
    swapping = false;
    swapT = 0.f;
    pendingSnapshot = -1;
    eventIndex = 0;
    eventStarted = false;
    playing = false;
}

void HeapAnimator::start(EventLog* eventLog, bool autoPlay, int startHeapSize)
{
    log = eventLog;
    initialHeapSize = startHeapSize;
    heapSize = startHeapSize;
    sortComplete = false;
    highlightA = -1;
    highlightB = -1;
    activeIndex = -1;
    swapping = false;
    swapT = 0.f;
    pendingSnapshot = -1;
    eventIndex = 0;
    eventTime = 0.f;
    eventStarted = false;
    playing = autoPlay;
    pauseAfterEvent = false;
    loadSnapshot(0);
}

// Sets up what the player should SEE while this event is playing. The array
// itself is only updated when the event finishes, so a swap can be shown as
// two cells sliding past each other instead of changing instantly.
void HeapAnimator::beginEvent(const Event& e, bool snap)
{
    pendingSnapshot = e.snapshotIndex;
    swapping = false;
    swapT = 0.f;

    if (e.type == Event::HEAP_SWAP)
    {
        highlightA = e.indexA;
        highlightB = e.indexB;
        swapping = true;
    }
    else if (e.type == Event::HEAPIFY_START)
    {
        activeIndex = e.indexA;
        highlightA = -1;
        highlightB = -1;
    }
    else if (e.type == Event::HEAPIFY_DONE)
    {
        activeIndex = -1;
        highlightA = -1;
        highlightB = -1;
    }
    else if (e.type == Event::EXTRACT_MAX)
    {
        activeIndex = 0;
        highlightA = 0;
        highlightB = -1;
    }
    else if (e.type == Event::MARK_SORTED)
    {
        activeIndex = -1;
        highlightA = e.indexA;
        highlightB = -1;
    }
    else if (e.type == Event::HEAP_BUILD)
    {
        highlightA = e.indexA;
        highlightB = -1;
    }
    else if (e.type == Event::HEAP_SORT_COMPLETE)
    {
        sortComplete = true;
        highlightA = -1;
        highlightB = -1;
        activeIndex = -1;
    }
    else if (e.type == Event::DONE)
    {
        highlightA = -1;
        highlightB = -1;
        activeIndex = -1;
    }

    // HEAP_SIZE_CHANGE is the only event that moves the heap/sorted boundary
    if (e.type == Event::HEAP_SIZE_CHANGE)
    {
        highlightA = -1;
        highlightB = -1;
    }

    if (snap)
    {
        finishEvent();
    }
}

void HeapAnimator::finishEvent()
{
    if (pendingSnapshot >= 0)
    {
        loadSnapshot(pendingSnapshot);
        pendingSnapshot = -1;
    }
    if (log != nullptr && eventIndex < log->size())
    {
        const Event& e = log->get(eventIndex);
        if (e.type == Event::HEAP_SIZE_CHANGE)
        {
            heapSize = e.newSize;
        }
        else if (e.type == Event::HEAP_BUILD || e.type == Event::HEAPIFY_START
                 || e.type == Event::EXTRACT_MAX)
        {
            heapSize = e.heapSize;
        }
        else if (e.type == Event::HEAP_SORT_COMPLETE)
        {
            heapSize = 0;
        }
    }
    swapping = false;
    swapT = 0.f;
}

void HeapAnimator::update(float dt)
{
    float duration = Theme::eventSeconds() / speed;

    if (swapping && swapT < 1.f)
    {
        swapT += dt / duration;
        if (swapT > 1.f)
        {
            swapT = 1.f;
        }
    }

    if (!playing || log == nullptr || eventIndex >= log->size())
    {
        return;
    }
    if (!eventStarted)
    {
        beginEvent(log->get(eventIndex), false);
        eventStarted = true;
        eventTime = 0.f;
    }
    eventTime += dt;
    if (eventTime >= duration)
    {
        finishEvent();
        eventIndex++;
        eventStarted = false;
        if (pauseAfterEvent)
        {
            playing = false;
            pauseAfterEvent = false;
        }
    }
}

void HeapAnimator::play()
{
    playing = true;
}

void HeapAnimator::pause()
{
    playing = false;
}

void HeapAnimator::togglePlay()
{
    playing = !playing;
}

bool HeapAnimator::isPlaying()
{
    return playing;
}

void HeapAnimator::stepForward()
{
    if (log == nullptr || eventIndex >= log->size())
    {
        return;
    }
    playing = true;
    pauseAfterEvent = true;
}

void HeapAnimator::stepBack()
{
    if (eventIndex > 0)
    {
        seek(eventIndex - 1);
    }
}

// Rebuilds the picture from the start, applying every earlier event at once.
void HeapAnimator::seek(int newIndex)
{
    if (log == nullptr)
    {
        return;
    }
    if (newIndex < 0)
    {
        newIndex = 0;
    }
    if (newIndex > log->size())
    {
        newIndex = log->size();
    }
    heapSize = initialHeapSize;
    sortComplete = false;
    highlightA = -1;
    highlightB = -1;
    activeIndex = -1;
    pendingSnapshot = -1;
    loadSnapshot(0);

    for (int i = 0; i < newIndex; i++)
    {
        eventIndex = i;          // finishEvent reads the event at eventIndex
        beginEvent(log->get(i), true);
    }
    eventIndex = newIndex;
    eventTime = 0.f;
    eventStarted = false;
    playing = false;
    swapping = false;
    swapT = 0.f;
}

void HeapAnimator::finishInstantly()
{
    if (log == nullptr)
    {
        return;
    }
    seek(log->size());
}

void HeapAnimator::setSpeed(float s)
{
    if (s < 0.25f)
    {
        s = 0.25f;
    }
    if (s > 4.f)
    {
        s = 4.f;
    }
    speed = s;
}

float HeapAnimator::getSpeed()
{
    return speed;
}

bool HeapAnimator::isFinished()
{
    return log == nullptr || eventIndex >= log->size();
}

int HeapAnimator::currentIndex()
{
    return eventIndex;
}

int HeapAnimator::totalEvents()
{
    return log != nullptr ? log->size() : 0;
}

float HeapAnimator::progress01()
{
    int total = totalEvents();
    if (total == 0)
    {
        return 1.f;
    }
    return (float)eventIndex / (float)total;
}

EventLog* HeapAnimator::getLog()
{
    return log;
}

int HeapAnimator::getCount()
{
    return count;
}

int HeapAnimator::getValue(int i)
{
    if (i < 0 || i >= count)
    {
        return 0;
    }
    return values[i];
}

int HeapAnimator::getHeapSize()
{
    return heapSize;
}

int HeapAnimator::getSortedFrom()
{
    return heapSize;
}

bool HeapAnimator::isSortComplete()
{
    return sortComplete;
}

int HeapAnimator::getHighlightA()
{
    return highlightA;
}

int HeapAnimator::getHighlightB()
{
    return highlightB;
}

int HeapAnimator::getActiveIndex()
{
    return activeIndex;
}

bool HeapAnimator::isSwapping()
{
    return swapping;
}

float HeapAnimator::getSwapProgress()
{
    return Theme::ease(swapT);
}

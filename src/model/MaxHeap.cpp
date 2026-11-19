#include "model/MaxHeap.h"
#include "core/TextUtil.h"

// `count` is how many values are stored; `heapSize` is how much of that array
// is still "the active heap". Heap Sort shrinks heapSize while count stays
// put, and everything from heapSize to count - 1 is the sorted region.

MaxHeap::MaxHeap()
{
    capacity = 256;
    data = new int[capacity];
    count = 0;
    heapSize = 0;
    log = nullptr;
    comparisons = 0;
    swaps = 0;
}

MaxHeap::~MaxHeap()
{
    delete[] data;
}

int MaxHeap::parentOf(int i)
{
    return (i - 1) / 2;
}

int MaxHeap::leftOf(int i)
{
    return 2 * i + 1;
}

int MaxHeap::rightOf(int i)
{
    return 2 * i + 2;
}

void MaxHeap::setEventLog(EventLog* eventLog)
{
    log = eventLog;
}

long long MaxHeap::getComparisons()
{
    return comparisons;
}

long long MaxHeap::getSwaps()
{
    return swaps;
}

void MaxHeap::resetCounters()
{
    comparisons = 0;
    swaps = 0;
}

int MaxHeap::get(int i)
{
    if (i < 0 || i >= count)
    {
        return 0;
    }
    return data[i];
}

int MaxHeap::totalCount()
{
    return count;
}

int MaxHeap::getHeapSize()
{
    return heapSize;
}

// Doubling growth, because std::vector is not allowed and a fixed array
// would cap the benchmarks at an arbitrary size.
void MaxHeap::growTo(int needed)
{
    if (needed <= capacity)
    {
        return;
    }
    int newCapacity = capacity;
    while (newCapacity < needed)
    {
        newCapacity = newCapacity * 2;
    }
    int* bigger = new int[newCapacity];
    for (int i = 0; i < count; i++)
    {
        bigger[i] = data[i];
    }
    delete[] data;
    data = bigger;
    capacity = newCapacity;
}

void MaxHeap::swapCells(int i, int j, const string& reason)
{
    int hold = data[i];
    data[i] = data[j];
    data[j] = hold;
    swaps++;
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::HEAP_SWAP;
    e.indexA = i;
    e.indexB = j;
    e.heapSize = heapSize;
    e.text = "Swap index " + TextUtil::intToText(i) + " and " + TextUtil::intToText(j)
           + " (" + reason + ")";
    e.snapshotIndex = log->saveArray(data, count);
    log->add(e);
}

void MaxHeap::logSizeChange(int oldHeapSize, int newHeapSize)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::HEAP_SIZE_CHANGE;
    e.oldSize = oldHeapSize;
    e.newSize = newHeapSize;
    e.heapSize = newHeapSize;
    e.text = "Active heap shrinks from " + TextUtil::intToText(oldHeapSize)
           + " to " + TextUtil::intToText(newHeapSize);
    e.snapshotIndex = log->saveArray(data, count);
    log->add(e);
}

void MaxHeap::logDone(const string& text)
{
    if (log == nullptr)
    {
        return;
    }
    Event e;
    e.type = Event::DONE;
    e.heapSize = heapSize;
    e.text = text;
    log->add(e);
}

void MaxHeap::load(const int a[], int n, int activeSize)
{
    if (n < 0)
    {
        n = 0;
    }
    growTo(n > 0 ? n : 1);
    for (int i = 0; i < n; i++)
    {
        data[i] = a[i];
    }
    count = n;
    if (activeSize < 0)
    {
        activeSize = 0;
    }
    if (activeSize > n)
    {
        activeSize = n;
    }
    heapSize = activeSize;
}

// Bottom-up construction: every index past the last parent is already a
// one-element heap, so only the parents need sifting. That is the O(n) build.
void MaxHeap::buildMaxHeap(const int a[], int n)
{
    load(a, n, n);
    if (log != nullptr)
    {
        Event e;
        e.type = Event::HEAP_BUILD;
        e.heapSize = heapSize;
        e.indexA = 0;
        e.indexB = count - 1;
        e.text = "Build a Max Heap from " + TextUtil::intToText(count) + " treasures";
        e.snapshotIndex = log->saveArray(data, count);
        log->add(e);
    }
    for (int i = count / 2 - 1; i >= 0; i--)
    {
        siftDown(i, heapSize);
    }
    logDone("Max Heap built");
}

// Walks the value at index i downward until both of its children are
// smaller, which is what restores the Max Heap property below i.
void MaxHeap::siftDown(int i, int activeSize)
{
    if (log != nullptr)
    {
        Event start;
        start.type = Event::HEAPIFY_START;
        start.indexA = i;
        start.heapSize = activeSize;
        start.text = "Sift down from index " + TextUtil::intToText(i);
        log->add(start);
    }

    int current = i;
    while (true)
    {
        int left = leftOf(current);
        int right = rightOf(current);
        int largest = current;
        if (left < activeSize)
        {
            comparisons++;
            if (data[left] > data[largest])
            {
                largest = left;
            }
        }
        if (right < activeSize)
        {
            comparisons++;
            if (data[right] > data[largest])
            {
                largest = right;
            }
        }
        if (largest == current)
        {
            break;
        }
        swapCells(current, largest, "sift down");
        current = largest;
    }

    if (log != nullptr)
    {
        Event doneEvent;
        doneEvent.type = Event::HEAPIFY_DONE;
        doneEvent.indexA = i;
        doneEvent.heapSize = activeSize;
        doneEvent.text = "Heap property restored below index " + TextUtil::intToText(i);
        log->add(doneEvent);
    }
}

// Re-heapifies whatever is in the array right now (Act 4.2 repair).
void MaxHeap::heapify()
{
    for (int i = heapSize / 2 - 1; i >= 0; i--)
    {
        siftDown(i, heapSize);
    }
    logDone("Heapify complete");
}

// Walks a freshly inserted value upward while it beats its parent.
void MaxHeap::siftUp(int i)
{
    int current = i;
    while (current > 0)
    {
        int parent = parentOf(current);
        comparisons++;
        if (data[current] <= data[parent])
        {
            break;
        }
        swapCells(current, parent, "sift up");
        current = parent;
    }
}

void MaxHeap::insert(int value)
{
    growTo(count + 1);
    // a new treasure always joins at the end of the active heap; anything
    // already sorted sits after it and must move one place along
    for (int i = count; i > heapSize; i--)
    {
        data[i] = data[i - 1];
    }
    data[heapSize] = value;
    count++;
    int inserted = heapSize;
    heapSize++;
    if (log != nullptr)
    {
        Event e;
        e.type = Event::HEAP_BUILD;
        e.indexA = inserted;
        e.value = value;
        e.heapSize = heapSize;
        e.text = "Insert " + TextUtil::intToText(value) + " at index "
               + TextUtil::intToText(inserted);
        e.snapshotIndex = log->saveArray(data, count);
        log->add(e);
    }
    siftUp(inserted);
    logDone("Inserted " + TextUtil::intToText(value));
}

// Swaps the root with the last active cell, shrinks the active heap by one
// and sifts the new root down. The old maximum stays in the array, just
// outside the heap, which is exactly the sorted region Heap Sort builds.
bool MaxHeap::extractMax(int& result)
{
    if (heapSize <= 0)
    {
        logDone("The heap is empty");
        return false;
    }
    result = data[0];
    if (log != nullptr)
    {
        Event e;
        e.type = Event::EXTRACT_MAX;
        e.indexA = 0;
        e.value = result;
        e.heapSize = heapSize;
        e.text = "Extract maximum " + TextUtil::intToText(result);
        log->add(e);
    }

    int last = heapSize - 1;
    if (last > 0)
    {
        swapCells(0, last, "root to the sorted region");
    }
    int oldHeapSize = heapSize;
    heapSize--;
    logSizeChange(oldHeapSize, heapSize);

    if (log != nullptr)
    {
        Event e;
        e.type = Event::MARK_SORTED;
        e.indexA = last;
        e.value = result;
        e.heapSize = heapSize;
        e.text = TextUtil::intToText(result) + " is locked into the sorted region";
        log->add(e);
    }
    if (heapSize > 1)
    {
        siftDown(0, heapSize);
    }
    logDone("Extracted " + TextUtil::intToText(result));
    return true;
}

// The player's move in Act 4: only a parent/child pair inside the active
// heap may be swapped, so an illegal click is reported instead of applied.
bool MaxHeap::playerSwap(int i, int j)
{
    if (i < 0 || j < 0 || i >= heapSize || j >= heapSize || i == j)
    {
        return false;
    }
    bool isParentChild = (i > 0 && parentOf(i) == j) || (j > 0 && parentOf(j) == i);
    if (!isParentChild)
    {
        return false;
    }
    swapCells(i, j, "player swap");
    logDone("Swapped index " + TextUtil::intToText(i) + " and " + TextUtil::intToText(j));
    return true;
}

bool MaxHeap::isMaxHeap()
{
    for (int i = 0; i < heapSize; i++)
    {
        int left = leftOf(i);
        int right = rightOf(i);
        if (left < heapSize && data[left] > data[i])
        {
            return false;
        }
        if (right < heapSize && data[right] > data[i])
        {
            return false;
        }
    }
    return true;
}

// Sorts a[] into sortedOut[] without touching the heap the GUI is showing,
// using a private working heap. Build is O(n) and each of the n extractions
// costs O(log n), so Heap Sort is O(n log n) overall.
void MaxHeap::heapSort(const int a[], int n, int sortedOut[])
{
    if (n <= 0)
    {
        return;
    }
    MaxHeap working;
    working.buildMaxHeap(a, n);
    for (int i = n - 1; i > 0; i--)
    {
        working.swapCells(0, i, "heap sort");
        working.heapSize--;
        working.siftDown(0, working.heapSize);
    }
    for (int i = 0; i < n; i++)
    {
        sortedOut[i] = working.data[i];
    }
    comparisons += working.comparisons;
    swaps += working.swaps;

    if (log != nullptr)
    {
        Event e;
        e.type = Event::HEAP_SORT_COMPLETE;
        e.heapSize = 0;
        e.text = "Heap Sort complete: " + TextUtil::intToText(n) + " treasures in order";
        e.snapshotIndex = log->saveArray(sortedOut, n);
        log->add(e);
        logDone("Final vault unlocked");
    }
}

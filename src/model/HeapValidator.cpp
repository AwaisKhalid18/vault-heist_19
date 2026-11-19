#include "model/HeapValidator.h"
#include "core/TextUtil.h"

// Walks the active heap and confirms no child beats its parent. The failure
// message names the two indices and their values so a level or a failing
// test says exactly what went wrong, the way TreeValidator does for trees.
bool HeapValidator::checkHeap(MaxHeap& h, string& error)
{
    error = "";
    int heapSize = h.getHeapSize();
    int total = h.totalCount();

    if (heapSize < 0 || heapSize > total)
    {
        error = "active heap size " + TextUtil::intToText(heapSize)
              + " does not fit in " + TextUtil::intToText(total) + " stored values";
        return false;
    }

    for (int i = 1; i < heapSize; i++)
    {
        int parent = h.parentOf(i);
        if (h.get(i) > h.get(parent))
        {
            error = "index " + TextUtil::intToText(i) + " (" + TextUtil::intToText(h.get(i))
                  + ") > parent index " + TextUtil::intToText(parent)
                  + " (" + TextUtil::intToText(h.get(parent)) + ")";
            return false;
        }
    }

    // The sorted region is everything past the active heap. Heap Sort fills
    // it from the back with successive maxima, so it must read in ascending
    // order no matter how many extractions have happened so far.
    for (int i = heapSize + 1; i < total; i++)
    {
        if (h.get(i) < h.get(i - 1))
        {
            error = "sorted region out of order at index " + TextUtil::intToText(i)
                  + " (" + TextUtil::intToText(h.get(i)) + " < "
                  + TextUtil::intToText(h.get(i - 1)) + ")";
            return false;
        }
    }
    return true;
}

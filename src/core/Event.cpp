#include "core/Event.h"

Event::Event()
{
    type = DONE;
    nodeId = -1;
    otherId = -1;
    parentId = -1;
    key = 0;
    value = 0;
    side = SIDE_NONE;
    cmp = CMP_EQUAL;
    rotation = ROT_NONE;
    mode = HL_VISITED;
    rotateLeft = false;
    oldValue = 0;
    newValue = 0;
    balanceFactor = 0;
    indexA = -1;
    indexB = -1;
    heapSize = 0;
    oldSize = 0;
    newSize = 0;
    snapshotIndex = -1;
    text = "";
}

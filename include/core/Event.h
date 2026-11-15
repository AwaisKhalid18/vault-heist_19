#pragma once

#include <string>

using namespace std;

// One saved node of a tree snapshot. -1 in any id field means "no node".
struct NodeState
{
    int id, key, leftId, rightId, parentId, height, size;
};

// One thing that happened inside a data structure during an operation.
// The GUI never guesses what the model did: it only plays these back.
class Event
{
public:
    // Event types (no enums allowed, so named int constants)
    static const int VISIT = 0, COMPARE = 1, INSERT = 2, UNLINK = 3, SWAP_VALUE = 4, ROTATE = 5,
                     UPDATE_HEIGHT = 6, UPDATE_SIZE = 7, BALANCE_ALERT = 8, HIGHLIGHT_SUBTREE = 9,
                     HEAP_BUILD = 10, HEAP_SWAP = 11, HEAPIFY_START = 12, HEAPIFY_DONE = 13,
                     EXTRACT_MAX = 14, MARK_SORTED = 15, HEAP_SIZE_CHANGE = 16,
                     HEAP_SORT_COMPLETE = 17, DONE = 18;
    // Sides
    static const int SIDE_NONE = 0, SIDE_LEFT = 1, SIDE_RIGHT = 2;
    // Comparison result: searched key compared to the room's key
    static const int CMP_LESS = 0, CMP_EQUAL = 1, CMP_GREATER = 2;
    // Rotation cases
    static const int ROT_NONE = 0, ROT_LL = 1, ROT_RR = 2, ROT_LR = 3, ROT_RL = 4;
    // Highlight modes
    static const int HL_VISITED = 0, HL_SKIPPED = 1, HL_RESULT = 2;

    Event();   // sets every field to a safe default (-1 / 0 / NONE)

    int type;
    int nodeId, otherId, parentId;   // otherId: SWAP_VALUE partner / new subtree root after ROTATE
    int key, value;
    int side, cmp, rotation, mode;
    bool rotateLeft;                 // which primitive rotation was applied
    int oldValue, newValue;          // UPDATE_HEIGHT / UPDATE_SIZE / SWAP_VALUE
    int balanceFactor;
    int indexA, indexB, heapSize, oldSize, newSize;   // heap events
    int snapshotIndex;               // -1, or the tree shape / array saved AFTER this event
    string text;                     // Event Log line, e.g. "Compare 40 < 50, go left"
};

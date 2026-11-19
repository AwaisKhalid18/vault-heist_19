#pragma once

#include "core/Limits.h"
#include "core/Event.h"

struct TreeNode;

// Holds every event of ONE operation, plus the tree-shape and array
// snapshots the animation needs to slide nodes to their new places.
// The arrays are allocated once in the constructor and reused.
class EventLog
{
public:
    EventLog();
    ~EventLog();

    void clear();
    int add(const Event& e);                 // returns the event's index
    int size();
    const Event& get(int i);

    // Stores the whole tree shape in preorder (done with a loop) and
    // returns the snapshot index, or -1 if the snapshot store is full.
    int saveTreeShape(const TreeNode* root);
    int saveArray(const int a[], int n);

    // Copies snapshot `index` into `out` and returns how many entries it has.
    int getTreeShape(int index, NodeState out[]);
    int getArray(int index, int out[]);
    int snapshotCount();

private:
    Event* events;
    int count;

    NodeState* shapes;                       // MAX_SNAPSHOTS blocks of MAX_NODES_GUI states
    int shapeSizes[Limits::MAX_SNAPSHOTS];
    int shapeCount;

    int* arrays;                             // MAX_SNAPSHOTS blocks of MAX_NODES_GUI ints
    int arraySizes[Limits::MAX_SNAPSHOTS];
    int arrayCount;
};

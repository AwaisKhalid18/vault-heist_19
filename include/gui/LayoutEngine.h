#pragma once

#include "core/Event.h"
#include "core/Limits.h"

// Where one node of a snapshot should sit on the canvas.
struct NodePlacement
{
    int id, key, parentId, depth, heightVal, sizeVal, bf;
    float x, y;
};

// Turns a tree snapshot (NodeState array) into (x, y) positions.
// x is the node's inorder position, y is its depth, so no two nodes
// ever overlap, even with 200+ nodes.
class LayoutEngine
{
public:
    // Fills out[] in the same order as shape[] and returns n.
    int layoutShape(const NodeState shape[], int n, NodePlacement out[]);

    // Positions for a complete tree stored in an array (Act 4 heaps).
    int layoutCompleteTree(int n, NodePlacement out[]);

private:
    int indexOfId(const NodeState shape[], int n, int id);
};

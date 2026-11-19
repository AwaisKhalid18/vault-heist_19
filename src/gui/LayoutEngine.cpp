#include "gui/LayoutEngine.h"
#include "gui/Theme.h"

int LayoutEngine::indexOfId(const NodeState shape[], int n, int id)
{
    if (id < 0)
    {
        return -1;
    }
    for (int i = 0; i < n; i++)
    {
        if (shape[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

int LayoutEngine::layoutShape(const NodeState shape[], int n, NodePlacement out[])
{
    if (n <= 0)
    {
        return 0;
    }

    // copy the data fields and find the root
    int rootIndex = -1;
    for (int i = 0; i < n; i++)
    {
        out[i].id = shape[i].id;
        out[i].key = shape[i].key;
        out[i].parentId = shape[i].parentId;
        out[i].heightVal = shape[i].height;
        out[i].sizeVal = shape[i].size;
        int leftIndex = indexOfId(shape, n, shape[i].leftId);
        int rightIndex = indexOfId(shape, n, shape[i].rightId);
        int leftHeight = leftIndex >= 0 ? shape[leftIndex].height : 0;
        int rightHeight = rightIndex >= 0 ? shape[rightIndex].height : 0;
        out[i].bf = leftHeight - rightHeight;
        out[i].depth = 0;
        out[i].x = 0.f;
        out[i].y = 0.f;
        if (shape[i].parentId == -1)
        {
            rootIndex = i;
        }
    }
    if (rootIndex == -1)
    {
        rootIndex = 0;
    }

    // iterative inorder: the visit order gives each node its x slot,
    // the recorded depth gives its y
    int stackIndex[Limits::MAX_NODES_GUI];
    int stackDepth[Limits::MAX_NODES_GUI];
    int stackTop = 0;
    int cur = rootIndex;
    int curDepth = 0;
    int slot = 0;

    float gapX = Theme::levelGapX();
    float gapY = Theme::levelGapY();

    while (cur != -1 || stackTop > 0)
    {
        while (cur != -1)
        {
            stackIndex[stackTop] = cur;
            stackDepth[stackTop] = curDepth;
            stackTop++;
            cur = indexOfId(shape, n, shape[cur].leftId);
            curDepth++;
        }
        stackTop--;
        cur = stackIndex[stackTop];
        curDepth = stackDepth[stackTop];

        out[cur].x = (float)slot * gapX;
        out[cur].y = (float)curDepth * gapY;
        out[cur].depth = curDepth;
        slot++;

        cur = indexOfId(shape, n, shape[cur].rightId);
        curDepth++;
    }

    // centre the tree around x = 0 so the camera fit looks symmetric
    float shift = (float)(slot - 1) * gapX / 2.f;
    for (int i = 0; i < n; i++)
    {
        out[i].x -= shift;
    }
    return n;
}

int LayoutEngine::layoutCompleteTree(int n, NodePlacement out[])
{
    if (n <= 0)
    {
        return 0;
    }
    float gapY = Theme::levelGapY();

    // find how many levels the complete tree has
    int levels = 0;
    int countAtLevel = 1;
    int total = 0;
    while (total < n)
    {
        total += countAtLevel;
        countAtLevel *= 2;
        levels++;
    }

    // the bottom level decides the full width
    int bottomSlots = 1;
    for (int i = 1; i < levels; i++)
    {
        bottomSlots *= 2;
    }
    float width = (float)bottomSlots * Theme::levelGapX() * 1.2f;

    for (int i = 0; i < n; i++)
    {
        // depth of index i in a complete tree
        int depth = 0;
        int firstAtDepth = 0;
        int sizeOfDepth = 1;
        while (i >= firstAtDepth + sizeOfDepth)
        {
            firstAtDepth += sizeOfDepth;
            sizeOfDepth *= 2;
            depth++;
        }
        int posInLevel = i - firstAtDepth;
        // spread this level evenly over the full width
        float cell = width / (float)sizeOfDepth;
        out[i].id = i;
        out[i].key = 0;
        out[i].parentId = i == 0 ? -1 : (i - 1) / 2;
        out[i].depth = depth;
        out[i].heightVal = 0;
        out[i].sizeVal = 0;
        out[i].bf = 0;
        out[i].x = cell * ((float)posInLevel + 0.5f) - width / 2.f;
        out[i].y = (float)depth * gapY;
    }
    return n;
}

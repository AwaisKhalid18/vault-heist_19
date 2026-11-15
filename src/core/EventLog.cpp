#include "core/EventLog.h"
#include "core/NodeStack.h"
#include "model/TreeNode.h"

EventLog::EventLog()
{
    events = new Event[Limits::MAX_EVENTS];
    count = 0;
    shapes = new NodeState[Limits::MAX_SNAPSHOTS * Limits::MAX_NODES_GUI];
    shapeCount = 0;
    arrays = new int[Limits::MAX_SNAPSHOTS * Limits::MAX_NODES_GUI];
    arrayCount = 0;
    for (int i = 0; i < Limits::MAX_SNAPSHOTS; i++)
    {
        shapeSizes[i] = 0;
        arraySizes[i] = 0;
    }
}

EventLog::~EventLog()
{
    delete[] events;
    delete[] shapes;
    delete[] arrays;
}

void EventLog::clear()
{
    count = 0;
    shapeCount = 0;
    arrayCount = 0;
}

int EventLog::add(const Event& e)
{
    if (count >= Limits::MAX_EVENTS)
    {
        return -1;   // log full: drop silently, the model still works
    }
    events[count] = e;
    count++;
    return count - 1;
}

int EventLog::size()
{
    return count;
}

const Event& EventLog::get(int i)
{
    return events[i];
}

int EventLog::saveTreeShape(const TreeNode* root)
{
    if (shapeCount >= Limits::MAX_SNAPSHOTS)
    {
        return -1;
    }
    NodeState* block = shapes + shapeCount * Limits::MAX_NODES_GUI;
    int n = 0;

    // Preorder walk with our own stack; the snapshot only reads the tree,
    // so the cast from const is safe and lets us reuse NodeStack.
    NodeStack stack;
    if (root != nullptr)
    {
        stack.push((TreeNode*)root);
    }
    while (!stack.isEmpty() && n < Limits::MAX_NODES_GUI)
    {
        TreeNode* node = stack.pop();
        block[n].id = node->id;
        block[n].key = node->key;
        block[n].leftId = node->left != nullptr ? node->left->id : -1;
        block[n].rightId = node->right != nullptr ? node->right->id : -1;
        block[n].parentId = node->parent != nullptr ? node->parent->id : -1;
        block[n].height = node->height;
        block[n].size = node->size;
        n++;
        // push right first so the left child is processed first (preorder)
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }

    shapeSizes[shapeCount] = n;
    shapeCount++;
    return shapeCount - 1;
}

int EventLog::saveArray(const int a[], int n)
{
    if (arrayCount >= Limits::MAX_SNAPSHOTS)
    {
        return -1;
    }
    if (n > Limits::MAX_NODES_GUI)
    {
        n = Limits::MAX_NODES_GUI;
    }
    int* block = arrays + arrayCount * Limits::MAX_NODES_GUI;
    for (int i = 0; i < n; i++)
    {
        block[i] = a[i];
    }
    arraySizes[arrayCount] = n;
    arrayCount++;
    return arrayCount - 1;
}

int EventLog::getTreeShape(int index, NodeState out[])
{
    if (index < 0 || index >= shapeCount)
    {
        return 0;
    }
    NodeState* block = shapes + index * Limits::MAX_NODES_GUI;
    int n = shapeSizes[index];
    for (int i = 0; i < n; i++)
    {
        out[i] = block[i];
    }
    return n;
}

int EventLog::getArray(int index, int out[])
{
    if (index < 0 || index >= arrayCount)
    {
        return 0;
    }
    int* block = arrays + index * Limits::MAX_NODES_GUI;
    int n = arraySizes[index];
    for (int i = 0; i < n; i++)
    {
        out[i] = block[i];
    }
    return n;
}

int EventLog::snapshotCount()
{
    return shapeCount;
}

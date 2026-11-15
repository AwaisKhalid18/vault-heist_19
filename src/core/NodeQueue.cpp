#include "core/NodeQueue.h"

NodeQueue::NodeQueue()
{
    capacity = 64;
    items = new TreeNode*[capacity];
    head = 0;
    count = 0;
}

NodeQueue::~NodeQueue()
{
    delete[] items;
}

void NodeQueue::grow()
{
    int newCapacity = capacity * 2;
    TreeNode** bigger = new TreeNode*[newCapacity];
    // copy the live elements to the start of the new array
    for (int i = 0; i < count; i++)
    {
        bigger[i] = items[head + i];
    }
    delete[] items;
    items = bigger;
    capacity = newCapacity;
    head = 0;
}

void NodeQueue::enqueue(TreeNode* node)
{
    if (head + count == capacity)
    {
        grow();
    }
    items[head + count] = node;
    count++;
}

TreeNode* NodeQueue::dequeue()
{
    if (count == 0)
    {
        return nullptr;
    }
    TreeNode* node = items[head];
    head++;
    count--;
    if (count == 0)
    {
        head = 0;   // queue empty, reuse the array from the start
    }
    return node;
}

bool NodeQueue::isEmpty()
{
    return count == 0;
}

int NodeQueue::size()
{
    return count;
}

void NodeQueue::clear()
{
    head = 0;
    count = 0;
}

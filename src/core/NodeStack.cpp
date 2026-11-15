#include "core/NodeStack.h"

NodeStack::NodeStack()
{
    capacity = 64;
    items = new TreeNode*[capacity];
    count = 0;
}

NodeStack::~NodeStack()
{
    delete[] items;
}

void NodeStack::grow()
{
    int newCapacity = capacity * 2;
    TreeNode** bigger = new TreeNode*[newCapacity];
    for (int i = 0; i < count; i++)
    {
        bigger[i] = items[i];
    }
    delete[] items;
    items = bigger;
    capacity = newCapacity;
}

void NodeStack::push(TreeNode* node)
{
    if (count == capacity)
    {
        grow();
    }
    items[count] = node;
    count++;
}

TreeNode* NodeStack::pop()
{
    if (count == 0)
    {
        return nullptr;
    }
    count--;
    return items[count];
}

TreeNode* NodeStack::peek()
{
    if (count == 0)
    {
        return nullptr;
    }
    return items[count - 1];
}

bool NodeStack::isEmpty()
{
    return count == 0;
}

int NodeStack::size()
{
    return count;
}

void NodeStack::clear()
{
    count = 0;
}

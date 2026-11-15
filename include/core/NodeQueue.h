#pragma once

struct TreeNode;

// A simple growing queue of TreeNode pointers, used for level-order traversal.
class NodeQueue
{
public:
    NodeQueue();
    ~NodeQueue();

    void enqueue(TreeNode* node);
    TreeNode* dequeue();        // returns nullptr if empty
    bool isEmpty();
    int size();
    void clear();

private:
    TreeNode** items;
    int head;                   // index of the front element
    int count;                  // elements currently stored
    int capacity;

    void grow();
};

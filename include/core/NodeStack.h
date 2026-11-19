#pragma once

struct TreeNode;

// A simple growing stack of TreeNode pointers, used for iterative traversals
// (the course forbids std::stack, and deep BSTs forbid recursion).
class NodeStack
{
public:
    NodeStack();
    ~NodeStack();

    void push(TreeNode* node);
    TreeNode* pop();            // returns nullptr if empty
    TreeNode* peek();           // returns nullptr if empty
    bool isEmpty();
    int size();
    void clear();

private:
    TreeNode** items;
    int count;
    int capacity;

    void grow();
};

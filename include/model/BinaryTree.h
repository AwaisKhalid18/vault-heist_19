#pragma once

// Act 1: a general binary tree. Its values carry no ordering at all - the
// shape is the whole lesson - so a tree is built from a pair of traversals
// rather than by inserting keys.

#include "model/TreeNode.h"
#include "core/EventLog.h"

class BinaryTree
{
public:
    BinaryTree();
    ~BinaryTree();
    void setEventLog(EventLog* eventLog);
    TreeNode* getRoot();

    // traversals: fill outIds and return how many ids were written
    int preorderRec(int outIds[], int maxCount);
    int preorderIter(int outIds[], int maxCount);
    int inorderRec(int outIds[], int maxCount);
    int inorderIter(int outIds[], int maxCount);
    int postorderRec(int outIds[], int maxCount);
    int postorderIter(int outIds[], int maxCount);
    int levelOrder(int outIds[], int maxCount);

    void buildPreIn(const int pre[], const int in[], int n);
    void buildPostIn(const int post[], const int in[], int n);

    int height();
    int diameter();
    bool lca(int idA, int idB, int& result);
    int pathTo(int id, int out[], int maxCount);
    void mirror();
    bool isSameTree(BinaryTree& other);
    bool isSymmetric();

private:
    TreeNode* root;
    int nextId;
    EventLog* log;
};

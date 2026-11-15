#pragma once

#include "model/BST.h"

// Self-balancing BST. After every insert or delete it walks back up the
// path and repairs any node whose balance factor leaves [-1, 1] with an
// LL, RR, LR or RL rotation (the Event::ROT_ constants).
class AVLTree : public BST
{
public:
    AVLTree();
    bool insert(int key);                        // BST insert, then rebalance going up
    bool remove(int key);                        // BST delete, then rebalance going up
    void setAutoRebalance(bool on);              // off = no rotations (manual-rotation levels)
    bool getAutoRebalance();
    bool rotateLeftAt(int nodeId);
    bool rotateRightAt(int nodeId);
    int  rebalanceAt(int nodeId);                // performs LL/RR/LR/RL, returns the ROT_ case used
    int  caseAt(int nodeId);                     // ROT_ case that would fix this node (ROT_NONE if fine)
    int  predictInsert(int key);                 // ROT_ case the next insert will cause; changes nothing
    int  balanceFactor(TreeNode* node);          // height(left) - height(right)
    bool kth(int k, int& result);                // k-th smallest using size, O(log n)
    int  rank(int key);                          // how many keys are <= key
    int  countInRange(int lo, int hi);           // rank(hi) - rank(lo - 1)
    bool isBalanced();
    int  unbalancedIds(int out[], int maxCount);
    int  deepestUnbalancedId();                  // -1 if balanced; repair this node first
    long long getRotations();
    int  getMaxRotationsLastOp();

private:
    bool autoRebalance;
    long long rotations;
    int rotationsThisOp;

    int caseOf(TreeNode* node);
    int rebalanceNode(TreeNode* node);
    void rebalanceUpFrom(TreeNode* node);
    void rotateLeftNode(TreeNode* x, int rotCase);
    void rotateRightNode(TreeNode* x, int rotCase);
    void logBalanceAlert(TreeNode* node, int bf);
};

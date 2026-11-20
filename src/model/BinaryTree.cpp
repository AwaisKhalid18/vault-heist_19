// TODO(B): Partner B replaces every stub body in this file.
#include "model/BinaryTree.h"

BinaryTree::BinaryTree()
{
    root = nullptr;
    nextId = 1;
    log = nullptr;
}

BinaryTree::~BinaryTree()
{
}

void BinaryTree::setEventLog(EventLog* eventLog)
{
    log = eventLog;
}

TreeNode* BinaryTree::getRoot()
{
    return root;
}

int BinaryTree::preorderRec(int outIds[], int maxCount) { return 0; }
int BinaryTree::preorderIter(int outIds[], int maxCount) { return 0; }
int BinaryTree::inorderRec(int outIds[], int maxCount) { return 0; }
int BinaryTree::inorderIter(int outIds[], int maxCount) { return 0; }
int BinaryTree::postorderRec(int outIds[], int maxCount) { return 0; }
int BinaryTree::postorderIter(int outIds[], int maxCount) { return 0; }
int BinaryTree::levelOrder(int outIds[], int maxCount) { return 0; }

void BinaryTree::buildPreIn(const int pre[], const int in[], int n) {}
void BinaryTree::buildPostIn(const int post[], const int in[], int n) {}

int BinaryTree::height() { return 0; }
int BinaryTree::diameter() { return 0; }
bool BinaryTree::lca(int idA, int idB, int& result) { return false; }
int BinaryTree::pathTo(int id, int out[], int maxCount) { return 0; }
void BinaryTree::mirror() {}
bool BinaryTree::isSameTree(BinaryTree& other) { return false; }
bool BinaryTree::isSymmetric() { return false; }

#pragma once

#include <string>

using namespace std;

// One room of the vault. Acts 1-3 all use this node.
// In BST/AVL `key` is the ordering key; Act 1 uses it as a plain value.
struct TreeNode
{
    int id;
    int key;
    string treasure;
    int height;         // leaf = 1, so an empty tree has height 0
    int size;           // nodes in this subtree, leaf = 1
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(int nodeId, int nodeKey)
    {
        id = nodeId;
        key = nodeKey;
        treasure = "";
        height = 1;
        size = 1;
        left = nullptr;
        right = nullptr;
        parent = nullptr;
    }
};

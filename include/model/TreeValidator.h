#pragma once

#include <string>
#include "model/TreeNode.h"

using namespace std;

class BST;
class AVLTree;

// Invariant checks, run after every operation in tests and when the App's
// debugValidate flag is on. Static functions only (functions, not
// variables, so no global state is involved).
class TreeValidator
{
public:
    // parent links, no cycles or shared nodes, and the stored height/size
    // of every node agrees with its children's stored values
    static bool checkStructure(TreeNode* root, string& error);

    // checkStructure + strict inorder ordering + node count == t.size()
    static bool checkBST(BST& t, string& error);

    // checkBST + |balance factor| <= 1 at every node
    static bool checkAVL(AVLTree& t, string& error);
};

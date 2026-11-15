#include "model/TreeValidator.h"
#include "model/BST.h"
#include "model/AVLTree.h"
#include "core/NodeStack.h"
#include "core/TextUtil.h"

bool TreeValidator::checkStructure(TreeNode* root, string& error)
{
    if (root == nullptr)
    {
        return true;
    }
    if (root->parent != nullptr)
    {
        error = "root has a parent pointer";
        return false;
    }

    // Walk every node once. A cycle or a shared node either breaks a
    // parent check or pushes the visit count past the hard cap.
    const int HARD_CAP = 1000000;
    int visited = 0;
    NodeStack stack;
    stack.push(root);
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        visited++;
        if (visited > HARD_CAP)
        {
            error = "cycle detected (visit count exploded)";
            return false;
        }

        if (node->left != nullptr)
        {
            if (node->left->parent != node)
            {
                error = "bad parent pointer at left child of " + TextUtil::intToText(node->key);
                return false;
            }
            stack.push(node->left);
        }
        if (node->right != nullptr)
        {
            if (node->right->parent != node)
            {
                error = "bad parent pointer at right child of " + TextUtil::intToText(node->key);
                return false;
            }
            stack.push(node->right);
        }

        // local height/size invariant; if it holds at every node, the
        // stored values are globally correct by induction from the leaves
        int leftHeight = node->left != nullptr ? node->left->height : 0;
        int rightHeight = node->right != nullptr ? node->right->height : 0;
        int wantHeight = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
        if (node->height != wantHeight)
        {
            error = "stored height of " + TextUtil::intToText(node->key) + " is "
                  + TextUtil::intToText(node->height) + ", expected " + TextUtil::intToText(wantHeight);
            return false;
        }
        int leftSize = node->left != nullptr ? node->left->size : 0;
        int rightSize = node->right != nullptr ? node->right->size : 0;
        int wantSize = 1 + leftSize + rightSize;
        if (node->size != wantSize)
        {
            error = "stored size of " + TextUtil::intToText(node->key) + " is "
                  + TextUtil::intToText(node->size) + ", expected " + TextUtil::intToText(wantSize);
            return false;
        }
    }
    return true;
}

bool TreeValidator::checkBST(BST& t, string& error)
{
    if (!checkStructure(t.getRoot(), error))
    {
        return false;
    }

    // iterative inorder must be strictly increasing, and the node count
    // must match what the tree believes its size is
    NodeStack stack;
    TreeNode* cur = t.getRoot();
    bool haveLast = false;
    int lastKey = 0;
    int visited = 0;
    while (cur != nullptr || !stack.isEmpty())
    {
        while (cur != nullptr)
        {
            stack.push(cur);
            cur = cur->left;
        }
        cur = stack.pop();
        visited++;
        if (haveLast && cur->key <= lastKey)
        {
            error = "ordering broken: " + TextUtil::intToText(cur->key)
                  + " after " + TextUtil::intToText(lastKey);
            return false;
        }
        lastKey = cur->key;
        haveLast = true;
        cur = cur->right;
    }
    if (visited != t.size())
    {
        error = "node count " + TextUtil::intToText(visited)
              + " does not match size() " + TextUtil::intToText(t.size());
        return false;
    }
    return true;
}

bool TreeValidator::checkAVL(AVLTree& t, string& error)
{
    if (!checkBST(t, error))
    {
        return false;
    }
    NodeStack stack;
    if (t.getRoot() != nullptr)
    {
        stack.push(t.getRoot());
    }
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        int bf = t.balanceFactor(node);
        if (bf > 1 || bf < -1)
        {
            error = "balance factor " + TextUtil::intToText(bf)
                  + " at " + TextUtil::intToText(node->key);
            return false;
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
    }
    return true;
}

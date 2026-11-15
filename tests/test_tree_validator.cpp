#include "TestRunner.h"
#include "model/AVLTree.h"
#include "model/TreeValidator.h"

// The validator must catch every way a tree can silently rot, so these
// tests deliberately damage hand-built trees and expect loud failures.
void runTreeValidatorTests(TestRunner& t)
{
    t.line("");
    t.line("-- TreeValidator --");
    string error;

    // a healthy tree passes all three checks
    AVLTree good;
    int keys[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++)
    {
        good.insert(keys[i]);
    }
    t.check(TreeValidator::checkStructure(good.getRoot(), error), "healthy structure passes");
    t.check(TreeValidator::checkBST(good, error), "healthy BST passes");
    t.check(TreeValidator::checkAVL(good, error), "healthy AVL passes");

    // wrong ordering: swap two keys behind the tree's back
    BST broken;
    broken.insert(50);
    broken.insert(30);
    broken.insert(70);
    broken.findByKey(30)->key = 90;   // 90 sits in the left subtree now
    t.check(!TreeValidator::checkBST(broken, error), "wrong order is caught: " + error);

    // wrong stored height
    BST badHeight;
    badHeight.insert(50);
    badHeight.insert(30);
    badHeight.findByKey(50)->height = 9;
    t.check(!TreeValidator::checkStructure(badHeight.getRoot(), error),
            "wrong height is caught: " + error);
    badHeight.findByKey(50)->height = 2;   // repair before the destructor

    // wrong stored size
    BST badSize;
    badSize.insert(50);
    badSize.insert(30);
    badSize.findByKey(50)->size = 7;
    t.check(!TreeValidator::checkStructure(badSize.getRoot(), error),
            "wrong size is caught: " + error);
    badSize.findByKey(50)->size = 2;

    // bad parent pointer
    BST badParent;
    badParent.insert(50);
    badParent.insert(30);
    badParent.insert(70);
    badParent.findByKey(30)->parent = badParent.findByKey(70);
    t.check(!TreeValidator::checkStructure(badParent.getRoot(), error),
            "bad parent pointer is caught: " + error);
    badParent.findByKey(30)->parent = badParent.findByKey(50);

    // cycle: a grandchild pointing back to an ancestor
    BST cycle;
    cycle.insert(50);
    cycle.insert(30);
    TreeNode* thirty = cycle.findByKey(30);
    thirty->left = cycle.getRoot();          // 30 -> 50 -> 30 -> ...
    t.check(!TreeValidator::checkStructure(cycle.getRoot(), error),
            "cycle is caught: " + error);
    thirty->left = nullptr;                   // repair before the destructor

    // balance violation caught only by the AVL check
    AVLTree lopsided;
    lopsided.setAutoRebalance(false);
    lopsided.insert(30);
    lopsided.insert(20);
    lopsided.insert(10);
    t.check(TreeValidator::checkBST(lopsided, error), "a chain is still a valid BST");
    t.check(!TreeValidator::checkAVL(lopsided, error), "the AVL check flags it: " + error);
}

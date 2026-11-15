#pragma once

#include <string>
#include "model/TreeNode.h"
#include "core/EventLog.h"

using namespace std;

// Binary Search Tree. Every function that walks a path uses a loop, never
// recursion, because a BST built from sorted keys has height n and deep
// recursion overflows the Windows stack.
class BST
{
public:
    // Delete cases (no enums allowed)
    static const int DEL_NOT_FOUND = 0, DEL_LEAF = 1, DEL_ONE_CHILD = 2, DEL_TWO_CHILDREN = 3;

    BST();
    virtual ~BST();
    void copyFrom(const BST& other);             // deep copy keeping ids and nextId (used for Undo)
    void setEventLog(EventLog* eventLog);        // nullptr = no events (benchmarks)

    virtual bool insert(int key);                // false if duplicate
    bool search(int key);                        // VISIT + COMPARE event per room
    virtual bool remove(int key);                // leaf / one child / two children (inorder successor)
    int  deleteCaseOf(int key);                  // returns a DEL_ constant, for level 2.3

    bool findMin(int& result);
    bool findMax(int& result);
    bool successor(int key, int& result);
    bool predecessor(int key, int& result);
    int  rangeQuery(int lo, int hi, int out[], int maxCount);   // pruned inorder, marks skipped subtrees
    bool kthSmallest(int k, int& result);        // O(n) inorder counter, k starts at 1
    bool lcaBST(int a, int b, int& result);

    int  height();
    int  size();
    bool isEmpty();
    bool contains(int key);                      // silent walk: no events, no counters
    TreeNode* getRoot();
    TreeNode* findById(int id);
    TreeNode* findByKey(int key);
    int  inorderKeys(int out[], int maxCount);
    int  searchPathLength(int key);              // rooms visited = optimal steps for a search
    void clear();
    long long getComparisons();
    void resetCounters();

protected:
    TreeNode* root;
    int count;
    int nextId;
    EventLog* log;
    long long comparisons;
    TreeNode* lastChangedParent;                 // where the last insert/remove changed the shape

    void updateUpwards(TreeNode* node);          // loop up to the root, fixing height and size

    // shared cores so AVLTree can rebalance before the DONE event is written
    bool insertCore(int key);
    bool removeCore(int key);

    // event helpers; all of them do nothing when log is nullptr
    void logVisit(TreeNode* node);
    void logCompare(TreeNode* node, int key, int cmpResult);
    void logDone(const string& text);
    void logHighlight(TreeNode* node, int mode, const string& text);
};

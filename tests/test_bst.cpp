#include "TestRunner.h"
#include "model/BST.h"
#include "model/TreeValidator.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// Builds the classic demo tree used by several tests:
//            50
//         /      \
//       30        70
//      /  \      /  \
//    20    40  60    80
static void buildDemoTree(BST& tree)
{
    tree.clear();
    int keys[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++)
    {
        tree.insert(keys[i]);
    }
}

void runBSTTests(TestRunner& t)
{
    t.line("");
    t.line("-- BST --");
    string error;

    // insert and search
    BST tree;
    buildDemoTree(tree);
    t.check(tree.size() == 7, "insert: size is 7");
    t.check(tree.height() == 3, "insert: height is 3 (leaf = 1)");
    t.check(tree.search(40) && tree.search(80) && !tree.search(99), "search finds and misses correctly");
    t.check(TreeValidator::checkBST(tree, error), "valid after inserts: " + error);

    // duplicates rejected
    t.check(!tree.insert(50), "duplicate root key rejected");
    t.check(!tree.insert(40), "duplicate inner key rejected");
    t.check(tree.size() == 7, "size unchanged after duplicates");

    // delete: leaf
    buildDemoTree(tree);
    t.check(tree.remove(20), "delete leaf 20");
    t.check(!tree.contains(20) && tree.size() == 6, "leaf really gone");
    t.check(TreeValidator::checkBST(tree, error), "valid after leaf delete: " + error);

    // delete: one left child
    buildDemoTree(tree);
    tree.remove(20);
    tree.insert(35);              // 40 now has one left child 35
    t.check(tree.deleteCaseOf(40) == BST::DEL_ONE_CHILD, "40 has one child");
    t.check(tree.remove(40), "delete node with one left child");
    t.check(tree.contains(35) && !tree.contains(40), "child 35 took 40's place");
    t.check(TreeValidator::checkBST(tree, error), "valid after one-left-child delete: " + error);

    // delete: one right child
    buildDemoTree(tree);
    tree.insert(45);              // 40 has one right child 45
    tree.remove(20);
    t.check(tree.remove(40), "delete node with one right child");
    t.check(tree.contains(45) && !tree.contains(40), "child 45 took 40's place");
    t.check(TreeValidator::checkBST(tree, error), "valid after one-right-child delete: " + error);

    // delete: two children (30 -> successor is 40)
    buildDemoTree(tree);
    t.check(tree.deleteCaseOf(30) == BST::DEL_TWO_CHILDREN, "30 has two children");
    t.check(tree.remove(30), "delete node with two children");
    t.check(!tree.contains(30) && tree.contains(40) && tree.contains(20),
            "successor kept, 30 gone");
    t.check(TreeValidator::checkBST(tree, error), "valid after two-children delete: " + error);

    // delete: root
    buildDemoTree(tree);
    t.check(tree.remove(50), "delete the root");
    t.check(!tree.contains(50) && tree.size() == 6, "root gone, others stay");
    t.check(TreeValidator::checkBST(tree, error), "valid after root delete: " + error);

    // delete: missing key
    t.check(!tree.remove(999), "deleting a missing key returns false");
    t.check(tree.deleteCaseOf(999) == BST::DEL_NOT_FOUND, "deleteCaseOf on missing key");

    // min / max
    buildDemoTree(tree);
    int value = 0;
    t.check(tree.findMin(value) && value == 20, "findMin is 20");
    t.check(tree.findMax(value) && value == 80, "findMax is 80");
    BST empty;
    t.check(!empty.findMin(value) && !empty.findMax(value), "min/max on empty tree fail");

    // successor / predecessor
    t.check(tree.successor(40, value) && value == 50, "successor of 40 is 50");
    t.check(tree.successor(45, value) && value == 50, "successor works for a missing key");
    t.check(!tree.successor(80, value), "80 has no successor");
    t.check(tree.predecessor(50, value) && value == 40, "predecessor of 50 is 40");
    t.check(!tree.predecessor(20, value), "20 has no predecessor");

    // range query
    int out[32];
    int found = tree.rangeQuery(30, 70, out, 32);
    t.check(found == 5, "range [30,70] holds 5 keys");
    bool sorted = true;
    for (int i = 1; i < found; i++)
    {
        if (out[i] <= out[i - 1])
        {
            sorted = false;
        }
    }
    t.check(sorted && out[0] == 30 && out[found - 1] == 70, "range result is sorted and correct");
    t.check(tree.rangeQuery(81, 99, out, 32) == 0, "empty range returns 0");

    // pruning: count visits with an event log; a pruned query of [75, 85]
    // must not enter the whole left half of the tree
    EventLog log;
    tree.setEventLog(&log);
    log.clear();
    tree.rangeQuery(75, 85, out, 32);
    int visits = 0;
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::VISIT)
        {
            visits++;
        }
    }
    tree.setEventLog(nullptr);
    t.check(visits <= 3, "range [75,85] visits at most 3 rooms (pruning works), visited "
            + TextUtil::intToText(visits));

    // kth smallest
    t.check(tree.kthSmallest(1, value) && value == 20, "1st smallest is 20");
    t.check(tree.kthSmallest(4, value) && value == 50, "4th smallest is 50");
    t.check(tree.kthSmallest(7, value) && value == 80, "7th smallest is 80");
    t.check(!tree.kthSmallest(8, value), "8th smallest does not exist");

    // lcaBST
    t.check(tree.lcaBST(20, 40, value) && value == 30, "LCA of 20 and 40 is 30");
    t.check(tree.lcaBST(40, 60, value) && value == 50, "LCA of 40 and 60 is 50");
    t.check(tree.lcaBST(30, 40, value) && value == 30, "LCA of 30 and its child 40 is 30");
    t.check(!tree.lcaBST(20, 999, value), "LCA with a missing key fails");

    // searchPathLength
    t.check(tree.searchPathLength(50) == 1, "path to the root is 1 room");
    t.check(tree.searchPathLength(20) == 3, "path to 20 is 3 rooms");

    // inorderKeys
    buildDemoTree(tree);
    int keys[16];
    int keyCount = tree.inorderKeys(keys, 16);
    bool inorderOk = keyCount == 7;
    for (int i = 1; i < keyCount; i++)
    {
        if (keys[i] <= keys[i - 1])
        {
            inorderOk = false;
        }
    }
    t.check(inorderOk, "inorderKeys returns all keys sorted");

    // deep tree: 10,000 sorted inserts must not crash (iterative code)
    BST deep;
    for (int i = 1; i <= 10000; i++)
    {
        deep.insert(i);
    }
    t.check(deep.height() == 10000, "10,000 sorted inserts give height 10,000");
    t.check(deep.search(10000), "search still works at depth 10,000");
    deep.clear();
    t.check(deep.isEmpty(), "clear() empties the deep tree without crashing");

    // copyFrom keeps ids and nextId
    buildDemoTree(tree);
    TreeNode* original = tree.findByKey(40);
    BST copy;
    copy.copyFrom(tree);
    TreeNode* cloned = copy.findByKey(40);
    t.check(cloned != nullptr && original != nullptr && cloned->id == original->id,
            "copyFrom keeps node ids");
    t.check(TreeValidator::checkBST(copy, error), "copy is a valid BST: " + error);
    copy.insert(90);
    tree.insert(90);
    TreeNode* newA = tree.findByKey(90);
    TreeNode* newB = copy.findByKey(90);
    t.check(newA != nullptr && newB != nullptr && newA->id == newB->id,
            "copyFrom keeps nextId (new nodes get the same id)");
}

#include "TestRunner.h"
#include "model/AVLTree.h"
#include "model/TreeValidator.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// Our own insertion sort for the reference array (no <algorithm> allowed).
static void sortInts(int a[], int n)
{
    for (int i = 1; i < n; i++)
    {
        int value = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > value)
        {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = value;
    }
}

// Returns the ROT_ case of the first ROTATE event in the log, or ROT_NONE.
static int firstRotationCase(EventLog& log)
{
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::ROTATE)
        {
            return log.get(i).rotation;
        }
    }
    return Event::ROT_NONE;
}

void runAVLTests(TestRunner& t)
{
    t.line("");
    t.line("-- AVL --");
    string error;
    EventLog log;

    // LL: inserting 30, 20, 10 forces a right rotation at 30
    AVLTree tree;
    tree.insert(30);
    tree.insert(20);
    tree.setEventLog(&log);
    log.clear();
    tree.insert(10);
    tree.setEventLog(nullptr);
    t.check(firstRotationCase(log) == Event::ROT_LL, "insert 30,20,10 triggers LL");
    t.check(tree.getRoot()->key == 20, "LL: 20 became the root");
    t.check(TreeValidator::checkAVL(tree, error), "LL: tree valid after repair: " + error);

    // RR: 10, 20, 30
    tree.clear();
    tree.insert(10);
    tree.insert(20);
    tree.setEventLog(&log);
    log.clear();
    tree.insert(30);
    tree.setEventLog(nullptr);
    t.check(firstRotationCase(log) == Event::ROT_RR, "insert 10,20,30 triggers RR");
    t.check(tree.getRoot()->key == 20, "RR: 20 became the root");
    t.check(TreeValidator::checkAVL(tree, error), "RR: tree valid after repair: " + error);

    // LR: 30, 10, 20
    tree.clear();
    tree.insert(30);
    tree.insert(10);
    tree.setEventLog(&log);
    log.clear();
    tree.insert(20);
    tree.setEventLog(nullptr);
    t.check(firstRotationCase(log) == Event::ROT_LR, "insert 30,10,20 triggers LR");
    t.check(tree.getRoot()->key == 20, "LR: 20 became the root");
    t.check(TreeValidator::checkAVL(tree, error), "LR: tree valid after repair: " + error);

    // RL: 10, 30, 20
    tree.clear();
    tree.insert(10);
    tree.insert(30);
    tree.setEventLog(&log);
    log.clear();
    tree.insert(20);
    tree.setEventLog(nullptr);
    t.check(firstRotationCase(log) == Event::ROT_RL, "insert 10,30,20 triggers RL");
    t.check(tree.getRoot()->key == 20, "RL: 20 became the root");
    t.check(TreeValidator::checkAVL(tree, error), "RL: tree valid after repair: " + error);

    // deletion rebalancing, including balance factor 0 on the heavy child:
    //        50                50
    //      /    \    delete   /    (the left child 30 has BF 0,
    //    30      70   70 ->  30     a single right rotation fixes it)
    //   /  \                /  \
    //  20    40            20   40
    tree.clear();
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.setEventLog(&log);
    log.clear();
    t.check(tree.remove(70), "delete 70 from the BF-0 example");
    tree.setEventLog(nullptr);
    t.check(firstRotationCase(log) == Event::ROT_LL,
            "deletion with BF 0 on the heavy child uses a single rotation");
    t.check(TreeValidator::checkAVL(tree, error), "valid after that deletion: " + error);

    // deletion that needs a double rotation
    tree.clear();
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(60);
    t.check(tree.remove(30), "delete 30 forcing an RL repair");
    t.check(TreeValidator::checkAVL(tree, error), "valid after RL deletion repair: " + error);

    // caseAt and predictInsert
    tree.clear();
    tree.setAutoRebalance(false);
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);
    TreeNode* root = tree.getRoot();
    t.check(tree.caseAt(root->id) == Event::ROT_LL, "caseAt sees the LL violation");
    tree.setAutoRebalance(true);
    tree.clear();
    tree.insert(30);
    tree.insert(20);
    t.check(tree.predictInsert(10) == Event::ROT_LL, "predictInsert foresees LL");
    t.check(tree.predictInsert(25) == Event::ROT_LR, "predictInsert foresees LR");
    t.check(tree.predictInsert(40) == Event::ROT_NONE, "predictInsert foresees no rotation");
    t.check(tree.size() == 2, "predictInsert changed nothing");
    t.check(TreeValidator::checkAVL(tree, error), "tree untouched by predictions: " + error);

    // manual rotations with auto-rebalance off
    tree.clear();
    tree.setAutoRebalance(false);
    tree.insert(30);
    tree.insert(20);
    tree.insert(10);
    t.check(!tree.isBalanced(), "manual mode: tree is unbalanced before repair");
    TreeNode* pivot = tree.findByKey(30);
    t.check(tree.rotateRightAt(pivot->id), "manual rotateRightAt works");
    t.check(tree.isBalanced(), "manual rotation fixed the balance");
    t.check(TreeValidator::checkBST(tree, error), "still a valid BST: " + error);
    t.check(!tree.rotateLeftAt(tree.findByKey(10)->id),
            "rotateLeftAt without a right child is refused");
    tree.setAutoRebalance(true);

    // rotateLeftAt then rotateRightAt are inverse operations
    tree.clear();
    tree.setAutoRebalance(false);
    int keys[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++)
    {
        tree.insert(keys[i]);
    }
    int before[16];
    int beforeCount = tree.inorderKeys(before, 16);
    tree.rotateLeftAt(tree.findByKey(50)->id);
    t.check(tree.getRoot()->key == 70, "rotateLeftAt lifted 70 to the root");
    tree.rotateRightAt(tree.findByKey(70)->id);
    t.check(tree.getRoot()->key == 50, "rotateRightAt restored 50");
    int after[16];
    int afterCount = tree.inorderKeys(after, 16);
    bool sameOrder = beforeCount == afterCount;
    for (int i = 0; i < beforeCount && sameOrder; i++)
    {
        if (before[i] != after[i])
        {
            sameOrder = false;
        }
    }
    t.check(sameOrder, "rotations never change the inorder sequence");
    tree.setAutoRebalance(true);

    // kth and rank match a sorted reference array through a seeded
    // 2,000-operation random run; heights/sizes validated as we go
    tree.clear();
    Random rng(424242);
    int reference[2200];
    int referenceCount = 0;
    bool kthOk = true;
    bool rankOk = true;
    bool validOk = true;
    bool countOk = true;
    for (int op = 0; op < 2000; op++)
    {
        int key = rng.nextInt(1, 500);
        if (rng.nextInt(0, 2) != 0)   // insert twice as often as delete
        {
            if (tree.insert(key))
            {
                reference[referenceCount] = key;
                referenceCount++;
            }
        }
        else
        {
            if (tree.remove(key))
            {
                int at = 0;
                while (reference[at] != key)
                {
                    at++;
                }
                reference[at] = reference[referenceCount - 1];
                referenceCount--;
            }
        }
        sortInts(reference, referenceCount);

        if (!TreeValidator::checkAVL(tree, error))
        {
            validOk = false;
        }
        if (tree.size() != referenceCount)
        {
            countOk = false;
        }
        // spot-check three ranks and three kth queries every operation
        for (int probe = 0; probe < 3; probe++)
        {
            int k = rng.nextInt(1, referenceCount > 0 ? referenceCount : 1);
            int result = 0;
            if (referenceCount > 0)
            {
                if (!tree.kth(k, result) || result != reference[k - 1])
                {
                    kthOk = false;
                }
                int queryKey = rng.nextInt(1, 500);
                int expected = 0;
                for (int i = 0; i < referenceCount; i++)
                {
                    if (reference[i] <= queryKey)
                    {
                        expected++;
                    }
                }
                if (tree.rank(queryKey) != expected)
                {
                    rankOk = false;
                }
            }
        }
    }
    t.check(validOk, "2,000-op run: validator passed after every operation");
    t.check(countOk, "2,000-op run: size always matched the reference");
    t.check(kthOk, "2,000-op run: kth always matched the sorted array");
    t.check(rankOk, "2,000-op run: rank always matched the sorted array");

    // countInRange against the reference
    int expectRange = 0;
    for (int i = 0; i < referenceCount; i++)
    {
        if (reference[i] >= 100 && reference[i] <= 300)
        {
            expectRange++;
        }
    }
    t.check(tree.countInRange(100, 300) == expectRange, "countInRange(100,300) matches");

    // stress: 10,000 seeded mixed operations with the validator each time
    tree.clear();
    Random stress(987654);
    bool stressOk = true;
    for (int op = 0; op < 10000; op++)
    {
        int key = stress.nextInt(1, 2000);
        if (stress.nextInt(0, 2) != 0)
        {
            tree.insert(key);
        }
        else
        {
            tree.remove(key);
        }
        if (!TreeValidator::checkAVL(tree, error))
        {
            stressOk = false;
            break;
        }
    }
    t.check(stressOk, "10,000-op stress run stayed valid: " + error);
    int logN = 1;
    int n = tree.size();
    while ((1 << logN) < n)
    {
        logN++;
    }
    t.check(tree.height() <= 2 * logN, "stress tree height is O(log n): height "
            + TextUtil::intToText(tree.height()) + " for n " + TextUtil::intToText(n));
}

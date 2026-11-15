#include "TestRunner.h"
#include "model/AVLTree.h"
#include "core/NodeStack.h"
#include "core/TextUtil.h"

// Compares a saved snapshot with the live tree, node by node (preorder).
static bool snapshotMatchesTree(EventLog& log, int snapshotIndex, TreeNode* root)
{
    NodeState shape[Limits::MAX_NODES_GUI];
    int n = log.getTreeShape(snapshotIndex, shape);

    NodeStack stack;
    if (root != nullptr)
    {
        stack.push(root);
    }
    int i = 0;
    while (!stack.isEmpty())
    {
        TreeNode* node = stack.pop();
        if (i >= n)
        {
            return false;
        }
        if (shape[i].id != node->id || shape[i].key != node->key
            || shape[i].height != node->height || shape[i].size != node->size)
        {
            return false;
        }
        int wantLeft = node->left != nullptr ? node->left->id : -1;
        int wantRight = node->right != nullptr ? node->right->id : -1;
        int wantParent = node->parent != nullptr ? node->parent->id : -1;
        if (shape[i].leftId != wantLeft || shape[i].rightId != wantRight
            || shape[i].parentId != wantParent)
        {
            return false;
        }
        i++;
        if (node->right != nullptr)
        {
            stack.push(node->right);
        }
        if (node->left != nullptr)
        {
            stack.push(node->left);
        }
    }
    return i == n;
}

void runTreeEventTests(TestRunner& t)
{
    t.line("");
    t.line("-- Events --");

    AVLTree tree;
    EventLog log;
    int keys[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++)
    {
        tree.insert(keys[i]);
    }

    // search: one VISIT and one COMPARE per room, equal to the comparisons
    tree.setEventLog(&log);
    log.clear();
    long long comparisonsBefore = tree.getComparisons();
    tree.search(40);
    long long comparisonsUsed = tree.getComparisons() - comparisonsBefore;
    int visitCount = 0;
    int compareCount = 0;
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::VISIT) { visitCount++; }
        if (log.get(i).type == Event::COMPARE) { compareCount++; }
    }
    t.check(visitCount == 3 && compareCount == 3,
            "search 40 writes 3 VISIT and 3 COMPARE events");
    t.check((long long)compareCount == comparisonsUsed,
            "COMPARE events equal the comparison counter");
    t.check(log.get(log.size() - 1).type == Event::DONE, "the batch ends with DONE");

    // compare directions carry the right side
    bool sidesOk = true;
    for (int i = 0; i < log.size(); i++)
    {
        const Event& e = log.get(i);
        if (e.type == Event::COMPARE && e.cmp == Event::CMP_LESS && e.side != Event::SIDE_LEFT)
        {
            sidesOk = false;
        }
        if (e.type == Event::COMPARE && e.cmp == Event::CMP_GREATER && e.side != Event::SIDE_RIGHT)
        {
            sidesOk = false;
        }
    }
    t.check(sidesOk, "COMPARE events carry the chosen side");

    // nullptr log: the model stays silent and never crashes
    tree.setEventLog(nullptr);
    log.clear();
    tree.insert(45);
    tree.remove(45);
    tree.search(60);
    t.check(log.size() == 0, "nullptr log: no events written");

    // insert writes an INSERT event whose snapshot matches the live tree
    tree.setEventLog(&log);
    log.clear();
    tree.insert(65);
    int insertSnapshot = -1;
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::INSERT)
        {
            insertSnapshot = log.get(i).snapshotIndex;
        }
    }
    t.check(insertSnapshot >= 0, "INSERT event carries a snapshot");
    t.check(snapshotMatchesTree(log, insertSnapshot, tree.getRoot()),
            "the INSERT snapshot matches the real tree");

    // rotation events carry the right case and primitive direction
    tree.setEventLog(nullptr);
    tree.clear();
    tree.insert(10);
    tree.insert(20);
    tree.setEventLog(&log);
    log.clear();
    tree.insert(30);   // RR: one left rotation
    bool foundRotate = false;
    bool rotateOk = false;
    int rotateSnapshot = -1;
    for (int i = 0; i < log.size(); i++)
    {
        const Event& e = log.get(i);
        if (e.type == Event::ROTATE)
        {
            foundRotate = true;
            rotateOk = e.rotation == Event::ROT_RR && e.rotateLeft;
            rotateSnapshot = e.snapshotIndex;
        }
    }
    t.check(foundRotate, "RR insert writes a ROTATE event");
    t.check(rotateOk, "the ROTATE event says RR via a left rotation");
    t.check(rotateSnapshot >= 0 && snapshotMatchesTree(log, rotateSnapshot, tree.getRoot()),
            "the ROTATE snapshot matches the real tree");

    // a BALANCE_ALERT precedes the rotation
    bool alertBeforeRotate = false;
    for (int i = 0; i + 1 < log.size(); i++)
    {
        if (log.get(i).type == Event::BALANCE_ALERT)
        {
            for (int j = i + 1; j < log.size(); j++)
            {
                if (log.get(j).type == Event::ROTATE)
                {
                    alertBeforeRotate = true;
                }
            }
        }
    }
    t.check(alertBeforeRotate, "BALANCE_ALERT comes before the ROTATE");

    // two-children delete writes SWAP_VALUE then UNLINK, both with snapshots
    tree.setEventLog(nullptr);
    tree.clear();
    int demo[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++)
    {
        tree.insert(demo[i]);
    }
    tree.setEventLog(&log);
    log.clear();
    tree.remove(50);
    int swapAt = -1;
    int unlinkAt = -1;
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::SWAP_VALUE) { swapAt = i; }
        if (log.get(i).type == Event::UNLINK) { unlinkAt = i; }
    }
    t.check(swapAt >= 0 && unlinkAt > swapAt, "two-children delete: SWAP_VALUE then UNLINK");
    t.check(log.get(unlinkAt).snapshotIndex >= 0
            && snapshotMatchesTree(log, log.get(unlinkAt).snapshotIndex, tree.getRoot()),
            "the UNLINK snapshot matches the real tree");
    tree.setEventLog(nullptr);

    // saveArray round trip (used by Partner B's heap)
    int numbers[5] = { 9, 4, 7, 1, 8 };
    int arrayIndex = log.saveArray(numbers, 5);
    int readBack[8];
    int readCount = log.getArray(arrayIndex, readBack);
    bool arrayOk = readCount == 5;
    for (int i = 0; i < 5 && arrayOk; i++)
    {
        if (readBack[i] != numbers[i])
        {
            arrayOk = false;
        }
    }
    t.check(arrayOk, "saveArray / getArray round trip");
}

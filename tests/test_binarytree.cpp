#include "TestRunner.h"
#include "model/BinaryTree.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// Act 1 model tests: the traversals (recursive against iterative),
// reconstruction round trips, height, diameter, LCA, paths and mirroring.

static TreeNode* findById(TreeNode* node, int id)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    if (node->id == id)
    {
        return node;
    }
    TreeNode* found = findById(node->left, id);
    return found != nullptr ? found : findById(node->right, id);
}

// Turns a list of node ids into the matching list of keys, so a test can be
// written against the values a human can see rather than internal ids.
static void idsToKeys(BinaryTree& tree, const int ids[], int count, int outKeys[])
{
    for (int i = 0; i < count; i++)
    {
        TreeNode* node = findById(tree.getRoot(), ids[i]);
        outKeys[i] = node != nullptr ? node->key : 0;
    }
}

static bool sameList(const int a[], const int b[], int n)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

static string listToText(const int a[], int n)
{
    string out = "";
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            out = out + " ";
        }
        out = out + TextUtil::intToText(a[i]);
    }
    return out;
}

// The worked example used all through these tests:
//
//            1
//          /   \
//         2     3
//        / \     \
//       4   5     6
//                /
//               7
//
// preorder  1 2 4 5 3 6 7
// inorder   4 2 5 1 3 7 6
// postorder 4 5 2 7 6 3 1
// level     1 2 3 4 5 6 7
static void buildExample(BinaryTree& tree)
{
    int pre[7] = { 1, 2, 4, 5, 3, 6, 7 };
    int in[7]  = { 4, 2, 5, 1, 3, 7, 6 };
    tree.buildPreIn(pre, in, 7);
}

static void testTraversals(TestRunner& t)
{
    BinaryTree tree;
    buildExample(tree);

    int ids[64];
    int keys[64];

    int expectedPre[7]   = { 1, 2, 4, 5, 3, 6, 7 };
    int expectedIn[7]    = { 4, 2, 5, 1, 3, 7, 6 };
    int expectedPost[7]  = { 4, 5, 2, 7, 6, 3, 1 };
    int expectedLevel[7] = { 1, 2, 3, 4, 5, 6, 7 };

    int n = tree.preorderRec(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedPre, 7),
            "preorderRec matches the hand-worked example (" + listToText(keys, n) + ")");

    n = tree.preorderIter(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedPre, 7), "preorderIter equals preorderRec");

    n = tree.inorderRec(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedIn, 7),
            "inorderRec matches the hand-worked example (" + listToText(keys, n) + ")");

    n = tree.inorderIter(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedIn, 7), "inorderIter equals inorderRec");

    n = tree.postorderRec(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedPost, 7),
            "postorderRec matches the hand-worked example (" + listToText(keys, n) + ")");

    n = tree.postorderIter(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedPost, 7), "postorderIter equals postorderRec");

    n = tree.levelOrder(ids, 64);
    idsToKeys(tree, ids, n, keys);
    t.check(n == 7 && sameList(keys, expectedLevel, 7),
            "levelOrder matches the hand-worked example (" + listToText(keys, n) + ")");
}

static void testEdgeCases(TestRunner& t)
{
    int ids[64];

    BinaryTree empty;
    t.check(empty.preorderRec(ids, 64) == 0, "empty tree: preorderRec writes nothing");
    t.check(empty.preorderIter(ids, 64) == 0, "empty tree: preorderIter writes nothing");
    t.check(empty.inorderIter(ids, 64) == 0, "empty tree: inorderIter writes nothing");
    t.check(empty.postorderIter(ids, 64) == 0, "empty tree: postorderIter writes nothing");
    t.check(empty.levelOrder(ids, 64) == 0, "empty tree: levelOrder writes nothing");
    t.check(empty.height() == 0, "empty tree has height 0");
    t.check(empty.diameter() == 0, "empty tree has diameter 0");
    t.check(empty.isSymmetric(), "empty tree counts as symmetric");
    t.check(empty.getRoot() == nullptr, "empty tree has no root");

    BinaryTree single;
    int pre1[1] = { 42 };
    int in1[1] = { 42 };
    single.buildPreIn(pre1, in1, 1);
    t.check(single.height() == 1, "single node has height 1 (leaf height = 1)");
    t.check(single.diameter() == 1, "single node has diameter 1");
    t.check(single.preorderRec(ids, 64) == 1, "single node: one id in preorder");
    t.check(single.isSymmetric(), "single node is symmetric");

    // a left-leaning chain of five rooms: height 5, diameter 5
    BinaryTree skewed;
    int preS[5] = { 5, 4, 3, 2, 1 };
    int inS[5]  = { 1, 2, 3, 4, 5 };
    skewed.buildPreIn(preS, inS, 5);
    t.check(skewed.height() == 5, "left chain of 5 has height 5");
    t.check(skewed.diameter() == 5, "left chain of 5 has diameter 5");

    // maxCount is respected even when the tree is larger
    BinaryTree tree;
    buildExample(tree);
    t.check(tree.preorderRec(ids, 3) == 3, "preorderRec stops at maxCount");
    t.check(tree.preorderIter(ids, 3) == 3, "preorderIter stops at maxCount");
    t.check(tree.inorderIter(ids, 3) == 3, "inorderIter stops at maxCount");
    t.check(tree.levelOrder(ids, 3) == 3, "levelOrder stops at maxCount");
}

static void testReconstruction(TestRunner& t)
{
    BinaryTree fromPre;
    buildExample(fromPre);

    // round trip: take this tree's traversals, rebuild from postorder +
    // inorder, and the two trees must be structurally identical
    int ids[64];
    int preKeys[64];
    int inKeys[64];
    int postKeys[64];
    int n = fromPre.preorderRec(ids, 64);
    idsToKeys(fromPre, ids, n, preKeys);
    fromPre.inorderRec(ids, 64);
    idsToKeys(fromPre, ids, n, inKeys);
    fromPre.postorderRec(ids, 64);
    idsToKeys(fromPre, ids, n, postKeys);

    BinaryTree fromPost;
    fromPost.buildPostIn(postKeys, inKeys, n);
    t.check(fromPre.isSameTree(fromPost), "postorder + inorder rebuilds the same tree");

    BinaryTree again;
    again.buildPreIn(preKeys, inKeys, n);
    t.check(fromPre.isSameTree(again), "preorder + inorder round trip is stable");
    t.check(again.height() == fromPre.height(), "round trip keeps the height");

    BinaryTree emptyBuild;
    int none[1] = { 0 };
    emptyBuild.buildPreIn(none, none, 0);
    t.check(emptyBuild.getRoot() == nullptr, "building from 0 rooms leaves an empty tree");

    // building twice in a row must free the first tree and start ids again
    BinaryTree rebuilt;
    buildExample(rebuilt);
    buildExample(rebuilt);
    t.check(rebuilt.preorderRec(ids, 64) == 7, "rebuilding replaces the previous tree");
}

static void testShapeQueries(TestRunner& t)
{
    BinaryTree tree;
    buildExample(tree);

    t.check(tree.height() == 4, "example tree has height 4 (1 -> 3 -> 6 -> 7)");
    // longest path is 4 -> 2 -> 1 -> 3 -> 6 -> 7, which is 6 rooms
    t.check(tree.diameter() == 6, "example tree has diameter 6 rooms");

    int ids[64];
    int n = tree.levelOrder(ids, 64);
    t.check(n == 7, "level order returns every room");

    // ids follow the preorder of the build, so id 1 is key 1, id 2 is key 2,
    // id 3 is key 4, id 4 is key 5, id 5 is key 3, id 6 is key 6, id 7 is key 7
    int result = -1;
    t.check(tree.lca(3, 4, result) && result == 2, "LCA of rooms 4 and 5 is room 2");
    t.check(tree.lca(3, 7, result) && result == 1, "LCA of rooms 4 and 7 is the root");
    t.check(tree.lca(2, 3, result) && result == 2, "LCA of a room and its own child is that room");
    t.check(!tree.lca(3, 99, result), "LCA reports false when a room is missing");
    t.check(!tree.lca(99, 100, result), "LCA reports false when both rooms are missing");

    int path[64];
    int length = tree.pathTo(7, path, 64);
    t.check(length == 4 && path[0] == 1 && path[3] == 7,
            "path to room 7 is 4 rooms long and starts at the root");
    t.check(tree.pathTo(1, path, 64) == 1, "path to the root is just the root");
    t.check(tree.pathTo(99, path, 64) == 0, "path to a missing room is empty");
}

static void testMirrorAndCompare(TestRunner& t)
{
    BinaryTree tree;
    buildExample(tree);
    BinaryTree copy;
    buildExample(copy);

    t.check(tree.isSameTree(copy), "two identical builds compare as the same tree");
    t.check(!tree.isSymmetric(), "the example tree is not symmetric");

    int ids[64];
    int beforeKeys[64];
    int n = tree.inorderRec(ids, 64);
    idsToKeys(tree, ids, n, beforeKeys);

    tree.mirror();
    t.check(!tree.isSameTree(copy), "mirroring changes the tree");

    int afterKeys[64];
    tree.inorderRec(ids, 64);
    idsToKeys(tree, ids, n, afterKeys);
    bool reversed = true;
    for (int i = 0; i < n; i++)
    {
        if (afterKeys[i] != beforeKeys[n - 1 - i])
        {
            reversed = false;
        }
    }
    t.check(reversed, "mirroring reverses the inorder traversal");

    tree.mirror();
    t.check(tree.isSameTree(copy), "mirroring twice returns to the original");
    t.check(tree.height() == copy.height(), "mirroring keeps the height");

    // a hand-built symmetric tree:  1 / (2, 2) with children 3,4 and 4,3
    BinaryTree symmetric;
    int preSym[7] = { 1, 2, 3, 4, 2, 4, 3 };
    int inSym[7]  = { 3, 2, 4, 1, 4, 2, 3 };
    symmetric.buildPreIn(preSym, inSym, 7);
    t.check(symmetric.isSymmetric(), "a mirror-image tree reports itself symmetric");
}

// Random shapes, checked against properties that must hold for any tree:
// the four traversals agree in length, recursive equals iterative, every
// room is reachable by a path, and mirroring twice is the identity.
static void testRandomShapes(TestRunner& t)
{
    Random rng(1773);
    bool allAgree = true;
    bool allPaths = true;
    bool allMirrors = true;
    bool allDiameters = true;

    for (int round = 0; round < 300; round++)
    {
        int n = rng.nextInt(1, 15);
        int values[16];
        for (int i = 0; i < n; i++)
        {
            values[i] = i + 1;          // distinct, so reconstruction is unique
        }
        rng.shuffle(values, n);

        // a random preorder for this inorder: pick a random root in each
        // range, then do the same for the two halves it creates
        int pre[16];
        int preCount = 0;
        int stackLo[32];
        int stackHi[32];
        int top = 0;
        stackLo[0] = 0;
        stackHi[0] = n - 1;
        top = 1;
        while (top > 0)
        {
            top--;
            int lo = stackLo[top];
            int hi = stackHi[top];
            if (lo > hi)
            {
                continue;
            }
            int split = rng.nextInt(lo, hi);
            pre[preCount] = values[split];
            preCount++;
            // push the right half first so the left half is handled next
            stackLo[top] = split + 1;
            stackHi[top] = hi;
            top++;
            stackLo[top] = lo;
            stackHi[top] = split - 1;
            top++;
        }

        BinaryTree tree;
        tree.buildPreIn(pre, values, n);

        int a[32];
        int b[32];
        int countA = tree.preorderRec(a, 32);
        int countB = tree.preorderIter(b, 32);
        if (countA != n || countB != n || !sameList(a, b, n))
        {
            allAgree = false;
        }
        countA = tree.inorderRec(a, 32);
        countB = tree.inorderIter(b, 32);
        if (countA != n || countB != n || !sameList(a, b, n))
        {
            allAgree = false;
        }
        countA = tree.postorderRec(a, 32);
        countB = tree.postorderIter(b, 32);
        if (countA != n || countB != n || !sameList(a, b, n))
        {
            allAgree = false;
        }
        if (tree.levelOrder(a, 32) != n)
        {
            allAgree = false;
        }

        // every room must have a root-to-room path no longer than the height
        tree.preorderRec(a, 32);
        for (int i = 0; i < n; i++)
        {
            int path[32];
            int length = tree.pathTo(a[i], path, 32);
            if (length < 1 || length > tree.height() || path[0] != tree.getRoot()->id)
            {
                allPaths = false;
            }
        }

        if (tree.diameter() < tree.height() || tree.diameter() > n)
        {
            allDiameters = false;
        }

        BinaryTree reference;
        reference.buildPreIn(pre, values, n);
        tree.mirror();
        tree.mirror();
        if (!tree.isSameTree(reference))
        {
            allMirrors = false;
        }
    }

    t.check(allAgree, "300 random shapes: recursive and iterative traversals always agree");
    t.check(allPaths, "300 random shapes: every room has a valid root path");
    t.check(allDiameters, "300 random shapes: height <= diameter <= room count");
    t.check(allMirrors, "300 random shapes: mirroring twice is the identity");
}

void runBinaryTreeTests(TestRunner& t)
{
    t.line("");
    t.line("-- BinaryTree (Act 1) --");
    testTraversals(t);
    testEdgeCases(t);
    testReconstruction(t);
    testShapeQueries(t);
    testMirrorAndCompare(t);
    testRandomShapes(t);
}

#include "TestRunner.h"
#include "model/BinaryTree.h"
#include "model/MaxHeap.h"
#include "model/HeapValidator.h"
#include "game/LevelLoader.h"
#include "game/LevelBuilderB.h"
#include "game/ScoreSystem.h"
#include "core/TextUtil.h"

// Act 1 and Act 4 level tests. The important ones prove each level is
// WINNABLE: a player who makes only optimal moves must finish inside the
// alarm capacity the level computes for itself. Without this a level can look
// fine and still be impossible, which is the thing that cannot be caught by
// playing it once.

// Plays Act 4's build levels perfectly: repeat bottom-up sift-downs, counting
// one alarm step per swap, which is exactly what the screen charges.
static int optimalBuildSteps(const int values[], int n)
{
    MaxHeap heap;
    heap.resetCounters();
    heap.buildMaxHeap(values, n);
    return (int)heap.getSwaps();
}

static bool loadLevel(const string& path, LevelDef& def)
{
    return LevelLoader::load(path, def);
}

static void testLevelFilesLoad(TestRunner& t)
{
    string paths[64];
    int levelCount = LevelLoader::loadLevelList("assets/levels/levels.txt", paths, 64);

    int act1 = 0;
    int act4 = 0;
    bool allGood = true;
    for (int i = 0; i < levelCount; i++)
    {
        LevelDef def;
        if (!LevelLoader::load(paths[i], def) || def.id == "" || def.type == "")
        {
            allGood = false;
            continue;
        }
        if (def.act == 1)
        {
            act1++;
        }
        if (def.act == 4)
        {
            act4++;
        }
        if (def.act == 1 || def.act == 4)
        {
            if (def.hintCount == 0 || def.briefing == "" || def.mission == "")
            {
                allGood = false;
            }
        }
    }
    t.check(allGood, "every Partner B level file loads with id, type, briefing and hints");
    t.check(act1 == 5, "the campaign lists all 5 Act 1 levels (found "
                       + TextUtil::intToText(act1) + ")");
    t.check(act4 == 5, "the campaign lists all 5 Act 4 levels (found "
                       + TextUtil::intToText(act4) + ")");
}

static void testAct1Levels(TestRunner& t)
{
    string ids[5] = { "1.1", "1.2", "1.3", "1.4", "1.5" };
    string paths[5] = { "assets/levels/act1/level_1_1.json",
                        "assets/levels/act1/level_1_2.json",
                        "assets/levels/act1/level_1_3.json",
                        "assets/levels/act1/level_1_4.json",
                        "assets/levels/act1/level_1_5.json" };

    bool allBuild = true;
    bool allSized = true;
    bool allWinnable = true;
    bool allReconstruct = true;

    for (int i = 0; i < 5; i++)
    {
        LevelDef def;
        if (!loadLevel(paths[i], def) || def.id != ids[i])
        {
            allBuild = false;
            continue;
        }

        BinaryTree tree;
        int pre[Limits::MAX_LEVEL_KEYS];
        int in[Limits::MAX_LEVEL_KEYS];
        int n = LevelBuilderB::buildTree(def, tree, pre, in);

        if (n != def.n)
        {
            allBuild = false;
        }
        // the spec asks for Act 1 trees of 6 to 15 rooms
        if (n < 6 || n > 15)
        {
            allSized = false;
        }

        // the generated preorder and inorder really do describe this tree
        int ids2[Limits::MAX_LEVEL_KEYS];
        int keys[Limits::MAX_LEVEL_KEYS];
        int count = tree.preorderRec(ids2, Limits::MAX_LEVEL_KEYS);
        if (count != n)
        {
            allReconstruct = false;
        }
        BinaryTree rebuilt;
        rebuilt.buildPreIn(pre, in, n);
        if (!tree.isSameTree(rebuilt))
        {
            allReconstruct = false;
        }

        // a perfect player walks the traversal once: n steps, and the level's
        // own capacity formula must leave room for that
        int optimal = n;
        if (def.type == "bt_lca")
        {
            optimal = tree.height() * 2;
        }
        if (optimal < 1)
        {
            optimal = 1;
        }
        if (ScoreSystem::alarmCapacity(optimal) <= optimal)
        {
            allWinnable = false;
        }
        (void)keys;
    }

    t.check(allBuild, "all 5 Act 1 levels build a tree of the size their file asks for");
    t.check(allSized, "every Act 1 tree has between 6 and 15 rooms, as the spec requires");
    t.check(allReconstruct, "every Act 1 level's preorder + inorder rebuilds its own tree");
    t.check(allWinnable, "every Act 1 level's alarm capacity exceeds its optimal path");
}

static void testAct1TreesAreReproducible(TestRunner& t)
{
    LevelDef def;
    if (!loadLevel("assets/levels/act1/level_1_1.json", def))
    {
        t.check(false, "level 1.1 loads");
        return;
    }
    BinaryTree first;
    BinaryTree second;
    int preA[Limits::MAX_LEVEL_KEYS];
    int inA[Limits::MAX_LEVEL_KEYS];
    int preB[Limits::MAX_LEVEL_KEYS];
    int inB[Limits::MAX_LEVEL_KEYS];
    int nA = LevelBuilderB::buildTree(def, first, preA, inA);
    int nB = LevelBuilderB::buildTree(def, second, preB, inB);

    t.check(nA == nB && first.isSameTree(second),
            "the same seed rebuilds exactly the same Act 1 vault");

    // distinct values, which reconstruction levels depend on
    bool distinct = true;
    for (int i = 0; i < nA; i++)
    {
        for (int j = i + 1; j < nA; j++)
        {
            if (inA[i] == inA[j])
            {
                distinct = false;
            }
        }
    }
    t.check(distinct, "Act 1 room values are distinct, so the rebuild is unique");
}

static void testAct4Levels(TestRunner& t)
{
    string ids[5] = { "4.1", "4.2", "4.3", "4.4", "4.5" };
    string paths[5] = { "assets/levels/act4/level_4_1.json",
                        "assets/levels/act4/level_4_2.json",
                        "assets/levels/act4/level_4_3.json",
                        "assets/levels/act4/level_4_4.json",
                        "assets/levels/act4/level_4_5.json" };

    bool allLoad = true;
    bool allWinnable = true;
    bool allValidStart = true;
    string error;

    for (int i = 0; i < 5; i++)
    {
        LevelDef def;
        if (!loadLevel(paths[i], def) || def.id != ids[i] || def.act != 4)
        {
            allLoad = false;
            continue;
        }

        int values[Limits::MAX_NODES_GUI];
        int n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
        if (n != def.n)
        {
            allLoad = false;
        }

        // what a perfect player spends, level by level, using the same
        // arithmetic the screens use
        int optimal = 1;
        if (def.type == "heap_build" || def.type == "heap_repair")
        {
            optimal = optimalBuildSteps(values, n);
        }
        else if (def.type == "heap_extract")
        {
            optimal = def.k > 0 && def.k < n ? def.k : n;
        }
        else if (def.type == "heap_race")
        {
            optimal = n;
        }
        else if (def.type == "heap_boss")
        {
            optimal = optimalBuildSteps(values, n) + n;
        }
        if (optimal < 1)
        {
            optimal = 1;
        }

        int capacity = ScoreSystem::alarmCapacity(optimal);
        if (capacity <= optimal)
        {
            allWinnable = false;
        }

        // playing it perfectly really does finish inside the capacity
        MaxHeap heap;
        heap.load(values, n, n);
        heap.resetCounters();
        int spent = 0;
        if (def.type == "heap_build" || def.type == "heap_repair")
        {
            heap.heapify();
            spent = (int)heap.getSwaps();
        }
        else if (def.type == "heap_boss")
        {
            heap.heapify();
            spent = (int)heap.getSwaps();
            while (heap.getHeapSize() > 0)
            {
                int taken = 0;
                heap.extractMax(taken);
                spent++;
            }
        }
        else
        {
            heap.heapify();
            int wanted = def.type == "heap_extract" && def.k > 0 && def.k < n ? def.k : n;
            for (int step = 0; step < wanted; step++)
            {
                int taken = 0;
                heap.extractMax(taken);
                spent++;
            }
        }
        if (spent > capacity)
        {
            allWinnable = false;
        }
        if (!HeapValidator::checkHeap(heap, error))
        {
            allValidStart = false;
        }
    }

    t.check(allLoad, "all 5 Act 4 level files load and build an array of the right size");
    t.check(allWinnable, "every Act 4 level can be finished inside its own alarm capacity");
    t.check(allValidStart, "every Act 4 level leaves a valid heap after a perfect play");
}

static void testAct4Generators(TestRunner& t)
{
    LevelDef def;
    def.n = 12;
    def.seed = 777;
    def.keyMin = 10;
    def.keyMax = 99;
    int values[Limits::MAX_NODES_GUI];

    def.generator = "heap";
    int n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
    MaxHeap alreadyHeap;
    alreadyHeap.load(values, n, n);
    t.check(alreadyHeap.isMaxHeap(), "the \"heap\" generator produces a valid Max Heap");

    def.generator = "disturbed";
    n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
    MaxHeap disturbed;
    disturbed.load(values, n, n);
    t.check(!disturbed.isMaxHeap(), "the \"disturbed\" generator really does break the heap");
    disturbed.heapify();
    t.check(disturbed.isMaxHeap(), "a disturbed vault can always be repaired");

    def.generator = "reverse";
    n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
    MaxHeap reversed;
    reversed.load(values, n, n);
    t.check(reversed.isMaxHeap(), "descending input is already a valid Max Heap");

    def.generator = "sorted";
    n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
    bool ascending = true;
    for (int i = 1; i < n; i++)
    {
        if (values[i] < values[i - 1])
        {
            ascending = false;
        }
    }
    t.check(ascending, "the \"sorted\" generator produces ascending input");

    def.generator = "random";
    n = LevelBuilderB::buildArray(def, values, Limits::MAX_NODES_GUI);
    int again[Limits::MAX_NODES_GUI];
    int n2 = LevelBuilderB::buildArray(def, again, Limits::MAX_NODES_GUI);
    bool same = n == n2;
    for (int i = 0; i < n && same; i++)
    {
        if (values[i] != again[i])
        {
            same = false;
        }
    }
    t.check(same, "the same seed rebuilds exactly the same Act 4 vault");
}

void runLevelBTests(TestRunner& t)
{
    t.line("");
    t.line("-- Levels (Acts 1 and 4) --");
    testLevelFilesLoad(t);
    testAct1Levels(t);
    testAct1TreesAreReproducible(t);
    testAct4Levels(t);
    testAct4Generators(t);
}

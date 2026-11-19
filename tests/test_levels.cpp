#include "TestRunner.h"
#include "model/AVLTree.h"
#include "model/TreeValidator.h"
#include "game/LevelLoader.h"
#include "game/LevelTreeBuilder.h"
#include "game/ScoreSystem.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// Counts the VISIT events an operation writes (one per room entered,
// which is exactly what the alarm meter charges for).
static int visitsOf(EventLog& log)
{
    int visits = 0;
    for (int i = 0; i < log.size(); i++)
    {
        if (log.get(i).type == Event::VISIT)
        {
            visits++;
        }
    }
    return visits;
}

// Repairs the whole tree the way the manual-rotation levels require
// (deepest violation first) and counts how often each case appears.
static void repairAndCount(AVLTree& tree, int caseCounts[5])
{
    int guard = 0;
    while (!tree.isBalanced() && guard < 50)
    {
        int nodeId = tree.deepestUnbalancedId();
        int rotCase = tree.rebalanceAt(nodeId);
        if (rotCase >= 0 && rotCase <= 4)
        {
            caseCounts[rotCase]++;
        }
        guard++;
    }
}

void runLevelTests(TestRunner& t)
{
    t.line("");
    t.line("-- Levels (Acts 2-3) --");
    string error;

    // every campaign level file loads
    // The campaign list also holds Partner B's acts, so this counts the Act
    // 2-3 entries rather than assuming they are the whole file.
    string paths[64];
    int levelCount = LevelLoader::loadLevelList("assets/levels/levels.txt", paths, 64);
    bool allLoad = levelCount >= 10;
    int act23Count = 0;
    for (int i = 0; i < levelCount; i++)
    {
        LevelDef def;
        if (!LevelLoader::load(paths[i], def) || def.id == "" || def.type == "")
        {
            allLoad = false;
        }
        else if (def.act == 2 || def.act == 3)
        {
            act23Count++;
        }
    }
    t.check(allLoad && act23Count == 10, "all 10 Act 2-3 level files load with id and type");

    // ---- 2.1 Search Heist: winnable within capacity ----
    LevelDef def;
    LevelLoader::load("assets/levels/act2/level_2_1.json", def);
    AVLTree tree;
    LevelTreeBuilder::build(def, tree, false);
    t.check(tree.size() == 12, "2.1 builds a 12-room vault");
    t.check(TreeValidator::checkBST(tree, error), "2.1 tree is a valid BST: " + error);
    int target = LevelTreeBuilder::pickDeepKey(tree);
    int optimal = tree.searchPathLength(target);
    int capacity = ScoreSystem::alarmCapacity(optimal);
    t.check(optimal >= 3, "2.1 target lies at least 3 rooms deep");
    t.check(optimal < capacity, "2.1 perfect play wins: optimal "
            + TextUtil::intToText(optimal) + " < capacity " + TextUtil::intToText(capacity));
    LevelOutcome fail;
    fail.completed = false;
    fail.alarmSteps = capacity;
    fail.capacity = capacity;
    fail.optimalSteps = optimal;
    t.check(ScoreSystem::computeStars(fail) == 0, "2.1 full alarm means 0 stars (retry)");

    // ---- 2.2 Stash Loot: three inserts fit in the capacity ----
    LevelLoader::load("assets/levels/act2/level_2_2.json", def);
    LevelTreeBuilder::build(def, tree, false);
    Random rng22(def.seed + 1);
    int insertOptimal = 0;
    for (int round = 0; round < 3; round++)
    {
        int key = LevelTreeBuilder::pickMissingKey(tree, def.keyMin, def.keyMax, rng22);
        insertOptimal += tree.searchPathLength(key) + 1;   // rooms passed + the new room
        t.check(tree.insert(key), "2.2 scripted insert " + TextUtil::intToText(key) + " works");
    }
    t.check(insertOptimal < ScoreSystem::alarmCapacity(insertOptimal),
            "2.2 perfect play fits in the alarm capacity");
    t.check(TreeValidator::checkBST(tree, error), "2.2 tree stays valid: " + error);

    // ---- 2.3 Steal and Collapse: every delete case exists ----
    LevelLoader::load("assets/levels/act2/level_2_3.json", def);
    LevelTreeBuilder::build(def, tree, false);
    t.check(LevelTreeBuilder::pickKeyWithDeleteCase(tree, BST::DEL_LEAF) != -1,
            "2.3 offers a leaf deletion");
    t.check(LevelTreeBuilder::pickKeyWithDeleteCase(tree, BST::DEL_ONE_CHILD) != -1,
            "2.3 offers a one-child deletion");
    t.check(LevelTreeBuilder::pickKeyWithDeleteCase(tree, BST::DEL_TWO_CHILDREN) != -1,
            "2.3 offers a two-children deletion");

    // the level's three perfect deletions stay within capacity
    EventLog log;
    tree.setEventLog(&log);
    int deleteOptimal = 0;
    int cases[3] = { BST::DEL_LEAF, BST::DEL_ONE_CHILD, BST::DEL_TWO_CHILDREN };
    for (int i = 0; i < 3; i++)
    {
        int key = LevelTreeBuilder::pickKeyWithDeleteCase(tree, cases[i]);
        log.clear();
        tree.remove(key);
        deleteOptimal += visitsOf(log);
    }
    tree.setEventLog(nullptr);
    t.check(deleteOptimal > 0 && deleteOptimal < ScoreSystem::alarmCapacity(deleteOptimal),
            "2.3 perfect deletions fit in the capacity");

    // ---- 2.4 Range Raid: pruning beats a full walk ----
    LevelLoader::load("assets/levels/act2/level_2_4.json", def);
    LevelTreeBuilder::build(def, tree, false);
    tree.setEventLog(&log);
    log.clear();
    int found[Limits::MAX_LEVEL_KEYS];
    int inRange = tree.rangeQuery(def.lo, def.hi, found, Limits::MAX_LEVEL_KEYS);
    int rangeOptimal = visitsOf(log);
    tree.setEventLog(nullptr);
    t.check(inRange >= 3, "2.4 range holds at least 3 treasures, holds "
            + TextUtil::intToText(inRange));
    t.check(rangeOptimal < tree.size(), "2.4 pruned query skips part of the vault ("
            + TextUtil::intToText(rangeOptimal) + " of " + TextUtil::intToText(tree.size()) + " rooms)");
    t.check(inRange <= rangeOptimal, "2.4 collecting the range fits inside the optimal walk");

    // ---- 2.5 Boss: corridor is winnable under the shrinking capacity ----
    LevelLoader::load("assets/levels/act2/level_2_5.json", def);
    LevelTreeBuilder::build(def, tree, false);
    t.check(tree.height() == def.n, "2.5 sorted inserts degenerate into a corridor of height n");
    int bossTarget = LevelTreeBuilder::pickDeepKey(tree);
    int bossOptimal = tree.searchPathLength(bossTarget);
    int bossCapacity = ScoreSystem::alarmCapacity(bossOptimal);
    // capacity shrinks by 1 every second step, so after `bossOptimal` perfect
    // steps the remaining capacity must still exceed the steps taken
    int shrink = bossOptimal / 2;
    t.check(bossOptimal < bossCapacity - shrink,
            "2.5 is winnable despite the shrinking capacity");

    // ---- 3.1 badges: the crafted tree has exactly 2 violations ----
    LevelLoader::load("assets/levels/act3/level_3_1.json", def);
    LevelTreeBuilder::build(def, tree, false);
    int unbalanced[Limits::MAX_NODES_GUI];
    int violations = tree.unbalancedIds(unbalanced, Limits::MAX_NODES_GUI);
    t.check(violations == 2, "3.1 has exactly 2 unbalanced rooms, found "
            + TextUtil::intToText(violations));

    // ---- 3.2 single rotations: LL and RR each appear at least twice ----
    int caseCounts[5] = { 0, 0, 0, 0, 0 };
    LevelLoader::load("assets/levels/act3/level_3_2.json", def);
    LevelTreeBuilder::build(def, tree, false);
    repairAndCount(tree, caseCounts);
    int scripted = def.keyCount - def.n;
    for (int i = 0; i < scripted; i++)
    {
        tree.setAutoRebalance(false);
        tree.insert(def.keys[def.n + i]);
        if (i % 2 == 1)
        {
            repairAndCount(tree, caseCounts);
        }
    }
    repairAndCount(tree, caseCounts);
    t.check(caseCounts[Event::ROT_LL] >= 2, "3.2 shows LL at least twice ("
            + TextUtil::intToText(caseCounts[Event::ROT_LL]) + ")");
    t.check(caseCounts[Event::ROT_RR] >= 2, "3.2 shows RR at least twice ("
            + TextUtil::intToText(caseCounts[Event::ROT_RR]) + ")");
    t.check(TreeValidator::checkAVL(tree, error), "3.2 fully repaired in the end: " + error);

    // ---- 3.3 double rotations: LR and RL each appear at least twice ----
    LevelLoader::load("assets/levels/act3/level_3_3.json", def);
    LevelTreeBuilder::build(def, tree, false);
    repairAndCount(tree, caseCounts);
    scripted = def.keyCount - def.n;
    for (int i = 0; i < scripted; i++)
    {
        tree.setAutoRebalance(false);
        tree.insert(def.keys[def.n + i]);
        if (i % 2 == 1)
        {
            repairAndCount(tree, caseCounts);
        }
    }
    repairAndCount(tree, caseCounts);
    t.check(caseCounts[Event::ROT_LR] >= 2, "3.3 shows LR at least twice ("
            + TextUtil::intToText(caseCounts[Event::ROT_LR]) + ")");
    t.check(caseCounts[Event::ROT_RL] >= 2, "3.3 shows RL at least twice ("
            + TextUtil::intToText(caseCounts[Event::ROT_RL]) + ")");

    // ---- 3.4 prediction: all four rotation cases occur in the script ----
    LevelLoader::load("assets/levels/act3/level_3_4.json", def);
    AVLTree predictTree;
    LevelTreeBuilder::build(def, predictTree, true);
    bool sawCase[5] = { false, false, false, false, false };
    for (int i = def.n; i < def.keyCount; i++)
    {
        int predicted = predictTree.predictInsert(def.keys[i]);
        sawCase[predicted] = true;
        predictTree.insert(def.keys[i]);
        if (!TreeValidator::checkAVL(predictTree, error))
        {
            t.check(false, "3.4 tree broke during the script: " + error);
        }
        if (predicted >= 0 && predicted <= 4)
        {
            caseCounts[predicted]++;
        }
    }
    t.check(sawCase[Event::ROT_LL] && sawCase[Event::ROT_RR]
            && sawCase[Event::ROT_LR] && sawCase[Event::ROT_RL],
            "3.4 script contains LL, RR, LR and RL predictions");

    // across Act 3 every rotation type appears at least twice
    t.check(caseCounts[Event::ROT_LL] >= 2 && caseCounts[Event::ROT_RR] >= 2
            && caseCounts[Event::ROT_LR] >= 2 && caseCounts[Event::ROT_RL] >= 2,
            "Act 3 overall: every rotation type appears at least twice");

    // ---- 3.5 boss: the k-th most valuable is reachable in O(log n) ----
    LevelLoader::load("assets/levels/act3/level_3_5.json", def);
    LevelTreeBuilder::build(def, tree, true);
    t.check(TreeValidator::checkAVL(tree, error), "3.5 tree is a valid AVL: " + error);
    int kthPosition = tree.size() - def.k + 1;   // k-th most valuable
    int kthKey = 0;
    t.check(tree.kth(kthPosition, kthKey), "3.5 the target treasure exists");
    int kthOptimal = tree.searchPathLength(kthKey);
    t.check(kthOptimal <= tree.height(), "3.5 the descent is at most the tree height");
    t.check(kthOptimal < ScoreSystem::alarmCapacity(kthOptimal),
            "3.5 perfect descent fits in the capacity");
}

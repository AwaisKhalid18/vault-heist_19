#include "TestRunner.h"
#include "core/NodeStack.h"
#include "core/NodeQueue.h"
#include "core/Random.h"
#include "core/TextUtil.h"
#include "game/ScoreSystem.h"
#include "game/LevelLoader.h"
#include "model/TreeNode.h"

// Shared-core tests: stack, queue, RNG, text helpers, scoring, JSON reader.
void runCoreTests(TestRunner& t)
{
    t.line("");
    t.line("-- Core --");

    // NodeStack grows past its initial capacity and keeps LIFO order
    NodeStack stack;
    TreeNode* probe = new TreeNode(1, 42);
    for (int i = 0; i < 200; i++)
    {
        stack.push(probe);
    }
    t.check(stack.size() == 200, "NodeStack grows to 200 entries");
    bool lifoOk = true;
    for (int i = 0; i < 200; i++)
    {
        if (stack.pop() != probe)
        {
            lifoOk = false;
        }
    }
    t.check(lifoOk && stack.isEmpty(), "NodeStack pops everything back");
    t.check(stack.pop() == nullptr, "NodeStack pop on empty returns nullptr");

    // NodeQueue keeps FIFO order across growth
    NodeQueue queue;
    TreeNode* nodes[3];
    for (int i = 0; i < 3; i++)
    {
        nodes[i] = new TreeNode(10 + i, i);
    }
    for (int round = 0; round < 50; round++)
    {
        queue.enqueue(nodes[0]);
        queue.enqueue(nodes[1]);
        queue.enqueue(nodes[2]);
    }
    bool fifoOk = true;
    for (int round = 0; round < 50; round++)
    {
        for (int i = 0; i < 3; i++)
        {
            if (queue.dequeue() != nodes[i])
            {
                fifoOk = false;
            }
        }
    }
    t.check(fifoOk && queue.isEmpty(), "NodeQueue keeps FIFO order through growth");
    delete probe;
    for (int i = 0; i < 3; i++)
    {
        delete nodes[i];
    }

    // Random is reproducible and stays in range
    Random a(1234);
    Random b(1234);
    bool sameSequence = true;
    bool inRange = true;
    for (int i = 0; i < 1000; i++)
    {
        int x = a.nextInt(5, 15);
        int y = b.nextInt(5, 15);
        if (x != y)
        {
            sameSequence = false;
        }
        if (x < 5 || x > 15)
        {
            inRange = false;
        }
    }
    t.check(sameSequence, "Random: same seed gives the same sequence");
    t.check(inRange, "Random: nextInt stays inside [lo, hi]");

    int perm[10];
    for (int i = 0; i < 10; i++)
    {
        perm[i] = i;
    }
    Random c(77);
    c.shuffle(perm, 10);
    bool allPresent = true;
    for (int want = 0; want < 10; want++)
    {
        bool found = false;
        for (int i = 0; i < 10; i++)
        {
            if (perm[i] == want)
            {
                found = true;
            }
        }
        if (!found)
        {
            allPresent = false;
        }
    }
    t.check(allPresent, "Random: shuffle keeps every element");

    // TextUtil round trip
    bool ok = false;
    t.check(TextUtil::intToText(-407) == "-407", "TextUtil: -407 to text");
    t.check(TextUtil::textToInt("1234", ok) == 1234 && ok, "TextUtil: text to 1234");
    TextUtil::textToInt("12x4", ok);
    t.check(!ok, "TextUtil: rejects broken numbers");

    // ScoreSystem formulas from the spec
    t.check(ScoreSystem::alarmCapacity(4) == 8, "alarmCapacity(4) = ceil(6)+2 = 8");
    t.check(ScoreSystem::alarmCapacity(5) == 10, "alarmCapacity(5) = ceil(7.5)+2 = 10");
    LevelOutcome o;
    o.completed = true;
    o.optimalSteps = 8;
    o.capacity = ScoreSystem::alarmCapacity(8);
    o.alarmSteps = 8;
    t.check(ScoreSystem::computeStars(o) == 3, "3 stars: steps == optimal, no penalties");
    o.alarmSteps = 10;
    o.penaltyCount = 1;
    t.check(ScoreSystem::computeStars(o) == 2, "2 stars: steps <= 1.25x optimal, 1 penalty");
    o.alarmSteps = 13;
    o.penaltyCount = 3;
    t.check(ScoreSystem::computeStars(o) == 1, "1 star: completed within capacity");
    o.completed = false;
    t.check(ScoreSystem::computeStars(o) == 0, "0 stars: alarm full");
    o.completed = true;
    o.alarmSteps = 8;
    o.penaltyCount = 0;
    o.hintUsed = true;
    t.check(ScoreSystem::computeStars(o) == 2, "hint caps the stars at 2");
    o.hintUsed = false;
    o.penaltyPoints = 50;
    o.bonuses = 100;
    t.check(ScoreSystem::computeScore(o) == 1000 - 80 - 50 + 100, "score formula");
    o.penaltyPoints = 5000;
    t.check(ScoreSystem::computeScore(o) == 0, "score never goes below 0");

    // LevelLoader reads one of the real level files
    LevelDef def;
    bool loaded = LevelLoader::load("assets/levels/act2/level_2_1.json", def);
    t.check(loaded, "LevelLoader: loads level_2_1.json");
    t.check(def.id == "2.1" && def.act == 2 && def.type == "bst_search",
            "LevelLoader: id, act and type are right");
    t.check(def.seed == 2101 && def.n == 12, "LevelLoader: numbers are right");
    t.check(def.hintCount >= 1, "LevelLoader: hints array was read");

    string paths[32];
    int pathCount = LevelLoader::loadLevelList("assets/levels/levels.txt", paths, 32);
    t.check(pathCount >= 10, "LevelLoader: levels.txt lists at least 10 levels");
}

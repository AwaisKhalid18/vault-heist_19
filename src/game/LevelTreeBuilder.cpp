#include "game/LevelTreeBuilder.h"
#include "model/AVLTree.h"
#include "core/Random.h"

int LevelTreeBuilder::startKeyCount(const LevelDef& def)
{
    if (def.generator == "explicit")
    {
        if (def.n > 0 && def.n < def.keyCount)
        {
            return def.n;
        }
        return def.keyCount;
    }
    return def.n;
}

void LevelTreeBuilder::build(const LevelDef& def, AVLTree& tree, bool autoRebalance)
{
    tree.clear();
    tree.setAutoRebalance(autoRebalance);
    // no events while building: the level starts with the tree already there
    EventLog* savedLog = nullptr;
    tree.setEventLog(savedLog);

    if (def.generator == "explicit")
    {
        int count = startKeyCount(def);
        for (int i = 0; i < count; i++)
        {
            tree.insert(def.keys[i]);
        }
        return;
    }

    if (def.generator == "sorted")
    {
        // sorted deliveries: 10, 20, 30, ... produces the degenerate corridor
        for (int i = 1; i <= def.n; i++)
        {
            tree.insert(i * 10);
        }
        return;
    }

    // "random": a seeded shuffle of distinct keys spread over [keyMin, keyMax]
    int pool[Limits::MAX_LEVEL_KEYS * 4];
    int poolSize = 0;
    int span = def.keyMax - def.keyMin + 1;
    int step = span / (def.n > 0 ? def.n : 1);
    if (step < 1)
    {
        step = 1;
    }
    int value = def.keyMin;
    while (poolSize < def.n && poolSize < Limits::MAX_LEVEL_KEYS * 4)
    {
        pool[poolSize] = value;
        poolSize++;
        value = value + step;
    }
    Random rng(def.seed);
    rng.shuffle(pool, poolSize);
    for (int i = 0; i < poolSize; i++)
    {
        tree.insert(pool[i]);
    }
}

int LevelTreeBuilder::pickDeepKey(AVLTree& tree)
{
    int keys[Limits::MAX_NODES_GUI];
    int n = tree.inorderKeys(keys, Limits::MAX_NODES_GUI);
    int bestKey = -1;
    int bestDepth = -1;
    for (int i = 0; i < n; i++)
    {
        int depth = tree.searchPathLength(keys[i]);
        if (depth > bestDepth)
        {
            bestDepth = depth;
            bestKey = keys[i];
        }
    }
    return bestKey;
}

int LevelTreeBuilder::pickMissingKey(AVLTree& tree, int keyMin, int keyMax, Random& rng)
{
    for (int attempt = 0; attempt < 500; attempt++)
    {
        int key = rng.nextInt(keyMin, keyMax);
        if (!tree.contains(key))
        {
            return key;
        }
    }
    // dense range: walk upward until a free key appears
    int key = keyMax + 1;
    while (tree.contains(key))
    {
        key++;
    }
    return key;
}

int LevelTreeBuilder::pickKeyWithDeleteCase(AVLTree& tree, int delCase)
{
    int keys[Limits::MAX_NODES_GUI];
    int n = tree.inorderKeys(keys, Limits::MAX_NODES_GUI);
    for (int i = 0; i < n; i++)
    {
        if (tree.deleteCaseOf(keys[i]) == delCase)
        {
            return keys[i];
        }
    }
    return -1;
}

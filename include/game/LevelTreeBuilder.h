#pragma once

#include "game/LevelDef.h"

class AVLTree;
class Random;

// Builds a level's starting tree from its definition. Lives outside the GUI
// so the unit tests can rebuild the exact same tree a level will show.
class LevelTreeBuilder
{
public:
    // Fills `tree` using def.generator:
    //   "explicit": inserts the first def.n keys (or all if n is 0)
    //   "sorted":   inserts 1..n multiplied by a step, in ascending order
    //   "random":   inserts a seeded random permutation of distinct keys
    // autoRebalance decides whether the tree self-balances while building.
    static void build(const LevelDef& def, AVLTree& tree, bool autoRebalance);

    // How many starting keys the definition produces (same rules as build).
    static int startKeyCount(const LevelDef& def);

    // The key whose search path is longest (ties: smallest key).
    // Levels use it as the search target, tests use it to prove levels
    // are winnable, so the choice must be deterministic.
    static int pickDeepKey(AVLTree& tree);

    // A key inside [keyMin, keyMax] that is NOT in the tree yet,
    // chosen with the given seeded generator.
    static int pickMissingKey(AVLTree& tree, int keyMin, int keyMax, Random& rng);

    // The smallest key whose delete case matches `delCase`, or -1.
    static int pickKeyWithDeleteCase(AVLTree& tree, int delCase);
};

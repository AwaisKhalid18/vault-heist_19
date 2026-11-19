#pragma once

#include "game/LevelDef.h"

class BinaryTree;

// Builds the starting data for Partner B's levels from a LevelDef, the way
// LevelTreeBuilder does for Partner A's. It lives outside the GUI so the unit
// tests can rebuild exactly what a level will show.
//
// Everything is driven by the level's `seed`, so a level looks the same every
// time it is played and can be graded.
class LevelBuilderB
{
public:
    // ---- Act 1: a random-shaped binary tree of distinct values ----
    // Fills `tree`, and writes the tree's preorder and inorder value lists
    // into preOut and inOut (level 1.2 shows those to the player).
    // Returns how many rooms were built.
    static int buildTree(const LevelDef& def, BinaryTree& tree, int preOut[], int inOut[]);

    // ---- Act 4: the starting array ----
    // def.generator picks the shape:
    //   "explicit"   the first def.n entries of def.keys
    //   "sorted"     ascending, the worst case for repeated insertion
    //   "reverse"    descending, which is already a valid Max Heap
    //   "duplicates" a small range of values, so ties are everywhere
    //   "heap"       a valid Max Heap built from random values
    //   "disturbed"  a valid Max Heap with a few pairs swapped back out of place
    //   anything else: random values
    // Returns how many values were written.
    static int buildArray(const LevelDef& def, int out[], int maxCount);

    // How many swaps bottom-up construction needs on this array. Levels use
    // it as their optimal step count, so it is computed, never typed in.
    static int optimalBuildSwaps(const int values[], int n);
};

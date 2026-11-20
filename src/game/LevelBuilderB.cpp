#include "game/LevelBuilderB.h"
#include "model/BinaryTree.h"
#include "model/MaxHeap.h"
#include "core/Random.h"
#include "core/Limits.h"

// Picks n distinct values in [keyMin, keyMax] using the level's seed.
static int pickDistinctValues(const LevelDef& def, Random& rng, int out[], int maxCount)
{
    int n = def.n;
    if (n > maxCount)
    {
        n = maxCount;
    }
    if (n < 0)
    {
        n = 0;
    }
    int lo = def.keyMin;
    int hi = def.keyMax;
    if (hi < lo)
    {
        hi = lo;
    }
    int span = hi - lo + 1;
    if (n > span)
    {
        n = span;
    }

    int count = 0;
    int guard = 0;
    while (count < n && guard < 20000)
    {
        int candidate = rng.nextInt(lo, hi);
        bool alreadyUsed = false;
        for (int i = 0; i < count; i++)
        {
            if (out[i] == candidate)
            {
                alreadyUsed = true;
            }
        }
        if (!alreadyUsed)
        {
            out[count] = candidate;
            count++;
        }
        guard++;
    }
    // if the random draw ran out of room, fill the rest in order
    int next = lo;
    while (count < n && next <= hi)
    {
        bool alreadyUsed = false;
        for (int i = 0; i < count; i++)
        {
            if (out[i] == next)
            {
                alreadyUsed = true;
            }
        }
        if (!alreadyUsed)
        {
            out[count] = next;
            count++;
        }
        next++;
    }
    return count;
}

// Any ordering of distinct values is a valid inorder list for some shape, so
// a random shape is made by picking a random root inside each inorder range
// and writing it to the preorder list. Done with an explicit stack of ranges
// rather than recursion, so the shape generator cannot overflow the stack.
static void randomPreorderFor(const int inorder[], int n, Random& rng, int preOut[])
{
    int stackLo[2 * Limits::MAX_LEVEL_KEYS + 4];
    int stackHi[2 * Limits::MAX_LEVEL_KEYS + 4];
    int top = 0;
    int written = 0;
    stackLo[top] = 0;
    stackHi[top] = n - 1;
    top++;

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
        preOut[written] = inorder[split];
        written++;
        // push the right half first so the left half is expanded next, which
        // is what makes the result a real preorder of the same tree
        stackLo[top] = split + 1;
        stackHi[top] = hi;
        top++;
        stackLo[top] = lo;
        stackHi[top] = split - 1;
        top++;
    }
}

int LevelBuilderB::buildTree(const LevelDef& def, BinaryTree& tree, int preOut[], int inOut[])
{
    Random rng(def.seed);
    int n = 0;

    if (def.generator == "explicit" && def.keyCount > 0)
    {
        n = def.n > 0 && def.n < def.keyCount ? def.n : def.keyCount;
        for (int i = 0; i < n; i++)
        {
            inOut[i] = def.keys[i];
        }
    }
    else
    {
        n = pickDistinctValues(def, rng, inOut, Limits::MAX_LEVEL_KEYS);
    }
    if (n <= 0)
    {
        return 0;
    }

    randomPreorderFor(inOut, n, rng, preOut);
    tree.buildPreIn(preOut, inOut, n);
    return n;
}

int LevelBuilderB::buildArray(const LevelDef& def, int out[], int maxCount)
{
    Random rng(def.seed);
    int n = def.n;
    if (n > maxCount)
    {
        n = maxCount;
    }
    if (n < 0)
    {
        n = 0;
    }

    if (def.generator == "explicit")
    {
        int count = def.keyCount;
        if (def.n > 0 && def.n < count)
        {
            count = def.n;
        }
        if (count > maxCount)
        {
            count = maxCount;
        }
        for (int i = 0; i < count; i++)
        {
            out[i] = def.keys[i];
        }
        return count;
    }

    if (def.generator == "sorted")
    {
        for (int i = 0; i < n; i++)
        {
            out[i] = def.keyMin + i * 3;
        }
        return n;
    }

    if (def.generator == "reverse")
    {
        for (int i = 0; i < n; i++)
        {
            out[i] = def.keyMin + (n - 1 - i) * 3;
        }
        return n;
    }

    if (def.generator == "duplicates")
    {
        int hi = def.keyMin + 4;
        if (def.keyMax < hi)
        {
            hi = def.keyMax;
        }
        for (int i = 0; i < n; i++)
        {
            out[i] = rng.nextInt(def.keyMin, hi) * 7;
        }
        return n;
    }

    for (int i = 0; i < n; i++)
    {
        out[i] = rng.nextInt(def.keyMin, def.keyMax);
    }

    if (def.generator == "heap" || def.generator == "disturbed")
    {
        MaxHeap builder;
        builder.buildMaxHeap(out, n);
        for (int i = 0; i < n; i++)
        {
            out[i] = builder.get(i);
        }
    }
    if (def.generator == "disturbed" && n > 2)
    {
        // break the heap in a few places, but only by swapping a parent with
        // one of its own children, so the repair is always a chain of legal
        // player swaps rather than something only a rebuild could fix
        int breaks = n / 4;
        if (breaks < 2)
        {
            breaks = 2;
        }
        for (int b = 0; b < breaks; b++)
        {
            int child = rng.nextInt(1, n - 1);
            int parent = (child - 1) / 2;
            int hold = out[child];
            out[child] = out[parent];
            out[parent] = hold;
        }
    }
    return n;
}

int LevelBuilderB::optimalBuildSwaps(const int values[], int n)
{
    MaxHeap probe;
    probe.resetCounters();
    probe.buildMaxHeap(values, n);
    return (int)probe.getSwaps();
}

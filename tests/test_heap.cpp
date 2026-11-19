#include "TestRunner.h"
#include "model/MaxHeap.h"
#include "model/HeapValidator.h"
#include "core/Random.h"
#include "core/TextUtil.h"

// Act 4 model tests: build, heapify, sift up and down, extraction, player
// swaps, Heap Sort and a long seeded random run validated after every step.

// A deliberately slow but obviously correct sort, written from scratch so the
// Heap Sort results have something independent to be compared against.
static void referenceSort(const int in[], int n, int out[])
{
    for (int i = 0; i < n; i++)
    {
        out[i] = in[i];
    }
    for (int i = 1; i < n; i++)
    {
        int value = out[i];
        int j = i - 1;
        while (j >= 0 && out[j] > value)
        {
            out[j + 1] = out[j];
            j--;
        }
        out[j + 1] = value;
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

static bool heapIsValid(MaxHeap& h)
{
    string error;
    return HeapValidator::checkHeap(h, error);
}

static void testIndexMaths(TestRunner& t)
{
    MaxHeap h;
    t.check(h.leftOf(0) == 1 && h.rightOf(0) == 2, "root's children are at index 1 and 2");
    t.check(h.leftOf(3) == 7 && h.rightOf(3) == 8, "index 3 has children 7 and 8");
    t.check(h.parentOf(1) == 0 && h.parentOf(2) == 0, "index 1 and 2 share parent 0");
    t.check(h.parentOf(7) == 3 && h.parentOf(8) == 3, "index 7 and 8 share parent 3");
    t.check(h.parentOf(h.leftOf(5)) == 5 && h.parentOf(h.rightOf(5)) == 5,
            "parentOf undoes leftOf and rightOf");
}

static void testBuildMaxHeap(TestRunner& t)
{
    MaxHeap h;

    h.buildMaxHeap(nullptr, 0);
    t.check(h.totalCount() == 0 && h.getHeapSize() == 0, "building from an empty array");
    t.check(h.isMaxHeap(), "an empty heap satisfies the heap property");
    t.check(heapIsValid(h), "validator accepts the empty heap");

    int one[1] = { 7 };
    h.buildMaxHeap(one, 1);
    t.check(h.totalCount() == 1 && h.get(0) == 7, "building from one element");
    t.check(h.isMaxHeap() && heapIsValid(h), "a single element is a valid Max Heap");

    int random[9] = { 3, 19, 1, 14, 8, 7, 10, 35, 2 };
    h.buildMaxHeap(random, 9);
    t.check(h.get(0) == 35, "the largest value ends up at the root");
    t.check(h.isMaxHeap() && heapIsValid(h), "random input builds a valid Max Heap");
    t.check(h.totalCount() == 9 && h.getHeapSize() == 9, "build keeps all nine values active");

    int reverse[8] = { 80, 70, 60, 50, 40, 30, 20, 10 };
    h.resetCounters();
    h.buildMaxHeap(reverse, 8);
    t.check(h.isMaxHeap() && heapIsValid(h), "an already-sorted descending array is already a heap");
    t.check(h.getSwaps() == 0, "building an already-valid heap needs no swaps");

    int ascending[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };
    h.buildMaxHeap(ascending, 8);
    t.check(h.get(0) == 80, "reverse-sorted input still lifts the maximum to the root");
    t.check(h.isMaxHeap() && heapIsValid(h), "reverse-sorted input builds a valid Max Heap");

    int duplicates[7] = { 5, 5, 5, 5, 5, 5, 5 };
    h.buildMaxHeap(duplicates, 7);
    t.check(h.isMaxHeap() && heapIsValid(h), "an all-duplicates array is a valid Max Heap");

    int negatives[6] = { -4, -17, -2, -30, -9, -1 };
    h.buildMaxHeap(negatives, 6);
    t.check(h.get(0) == -1 && h.isMaxHeap(), "negative values heap correctly");
}

static void testSiftAndHeapify(TestRunner& t)
{
    // a heap with one value out of place at the root
    int broken[7] = { 2, 70, 60, 10, 30, 40, 50 };
    MaxHeap h;
    h.load(broken, 7, 7);
    t.check(!h.isMaxHeap(), "the disturbed array is not a Max Heap yet");

    h.siftDown(0, h.getHeapSize());
    t.check(h.isMaxHeap() && heapIsValid(h), "siftDown at the root repairs the heap");
    t.check(h.get(0) == 70, "siftDown lifts the larger child to the root");

    int scrambled[10] = { 4, 9, 1, 20, 3, 15, 2, 8, 11, 6 };
    MaxHeap repair;
    repair.load(scrambled, 10, 10);
    repair.heapify();
    t.check(repair.isMaxHeap() && heapIsValid(repair), "heapify repairs a scrambled array");
    t.check(repair.get(0) == 20, "heapify brings the maximum to the root");

    // siftUp is what insert relies on
    MaxHeap up;
    int base[4] = { 50, 40, 30, 20 };
    up.load(base, 4, 4);
    up.insert(99);
    t.check(up.get(0) == 99, "a new maximum sifts all the way up to the root");
    t.check(up.totalCount() == 5 && up.getHeapSize() == 5, "insert grows both counts");
    t.check(up.isMaxHeap() && heapIsValid(up), "the heap is still valid after an insert");

    up.insert(1);
    t.check(up.get(0) == 99 && up.isMaxHeap(), "a small insert does not disturb the root");

    MaxHeap grow;
    grow.buildMaxHeap(nullptr, 0);
    for (int i = 0; i < 400; i++)
    {
        grow.insert(i);
    }
    t.check(grow.totalCount() == 400 && grow.get(0) == 399,
            "the array grows past its first capacity");
    t.check(grow.isMaxHeap() && heapIsValid(grow), "400 inserts leave a valid Max Heap");
}

static void testExtractMax(TestRunner& t)
{
    MaxHeap h;
    int result = -1;
    t.check(!h.extractMax(result), "extracting from an empty heap reports false");

    int values[9] = { 3, 19, 1, 14, 8, 7, 10, 35, 2 };
    h.buildMaxHeap(values, 9);

    bool alwaysMax = true;
    bool alwaysValid = true;
    bool descending = true;
    int previous = 0;
    for (int i = 0; i < 9; i++)
    {
        int before = h.get(0);
        if (!h.extractMax(result) || result != before)
        {
            alwaysMax = false;
        }
        if (!h.isMaxHeap() || !heapIsValid(h))
        {
            alwaysValid = false;
        }
        if (i > 0 && result > previous)
        {
            descending = false;
        }
        previous = result;
    }
    t.check(alwaysMax, "extractMax always returns the current root");
    t.check(alwaysValid, "the remaining heap stays valid after every extraction");
    t.check(descending, "successive extractions come out in descending order");
    t.check(h.getHeapSize() == 0, "the active heap is empty once everything is extracted");
    t.check(h.totalCount() == 9, "the extracted values stay in the array as the sorted region");

    // the sorted region left behind must be the input in ascending order
    int reference[9];
    referenceSort(values, 9, reference);
    int produced[9];
    for (int i = 0; i < 9; i++)
    {
        produced[i] = h.get(i);
    }
    t.check(sameList(produced, reference, 9),
            "repeated extraction leaves the array sorted ascending");

    t.check(!h.extractMax(result), "extracting past the end reports false");
}

static void testPlayerSwap(TestRunner& t)
{
    int values[7] = { 50, 40, 30, 20, 10, 25, 15 };
    MaxHeap h;
    h.load(values, 7, 7);

    t.check(h.playerSwap(0, 1), "swapping a parent with its child is allowed");
    t.check(h.get(0) == 40 && h.get(1) == 50, "the swap really exchanged the two cells");
    t.check(h.playerSwap(1, 0), "the swap works in either argument order");
    t.check(h.get(0) == 50, "swapping back restores the original");

    t.check(!h.playerSwap(1, 2), "swapping two siblings is rejected");
    t.check(!h.playerSwap(0, 5), "swapping a room with its grandchild is rejected");
    t.check(!h.playerSwap(3, 3), "swapping a cell with itself is rejected");
    t.check(!h.playerSwap(-1, 0), "a negative index is rejected");
    t.check(!h.playerSwap(0, 99), "an index past the heap is rejected");

    h.load(values, 7, 3);
    t.check(!h.playerSwap(1, 3), "a cell outside the active heap cannot be swapped");
}

static void testHeapSort(TestRunner& t)
{
    MaxHeap h;
    int sorted[32];
    int reference[32];

    h.heapSort(nullptr, 0, sorted);
    t.check(true, "heapSort on an empty array does nothing and does not crash");

    int one[1] = { 9 };
    h.heapSort(one, 1, sorted);
    t.check(sorted[0] == 9, "heapSort of a single element");

    int random[10] = { 4, 9, 1, 20, 3, 15, 2, 8, 11, 6 };
    referenceSort(random, 10, reference);
    h.heapSort(random, 10, sorted);
    t.check(sameList(sorted, reference, 10), "heapSort matches the reference on random input");

    int ascending[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    referenceSort(ascending, 8, reference);
    h.heapSort(ascending, 8, sorted);
    t.check(sameList(sorted, reference, 8), "heapSort handles already-sorted input");

    int descending[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };
    referenceSort(descending, 8, reference);
    h.heapSort(descending, 8, sorted);
    t.check(sameList(sorted, reference, 8), "heapSort handles reverse-sorted input");

    int duplicates[9] = { 5, 1, 5, 3, 1, 5, 3, 1, 3 };
    referenceSort(duplicates, 9, reference);
    h.heapSort(duplicates, 9, sorted);
    t.check(sameList(sorted, reference, 9), "heapSort handles duplicate-heavy input");

    int negatives[7] = { -3, 12, 0, -40, 7, -1, 7 };
    referenceSort(negatives, 7, reference);
    h.heapSort(negatives, 7, sorted);
    t.check(sameList(sorted, reference, 7), "heapSort handles negative values");

    // sorting must not disturb the live heap the GUI is showing
    int live[6] = { 60, 50, 40, 30, 20, 10 };
    MaxHeap shown;
    shown.buildMaxHeap(live, 6);
    int before[6];
    for (int i = 0; i < 6; i++)
    {
        before[i] = shown.get(i);
    }
    int output[6];
    shown.heapSort(live, 6, output);
    int after[6];
    for (int i = 0; i < 6; i++)
    {
        after[i] = shown.get(i);
    }
    t.check(sameList(before, after, 6), "heapSort leaves the live heap untouched");
    t.check(shown.getHeapSize() == 6, "heapSort leaves the live active heap size untouched");

    // random sizes against the reference sort
    Random rng(4401);
    bool allSorted = true;
    for (int round = 0; round < 200; round++)
    {
        int n = rng.nextInt(1, 30);
        int input[32];
        for (int i = 0; i < n; i++)
        {
            input[i] = rng.nextInt(-99, 99);
        }
        referenceSort(input, n, reference);
        h.heapSort(input, n, sorted);
        if (!sameList(sorted, reference, n))
        {
            allSorted = false;
        }
    }
    t.check(allSorted, "200 random arrays: heapSort always matches the reference sort");
}

static void testCounters(TestRunner& t)
{
    MaxHeap h;
    int values[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };

    h.resetCounters();
    h.buildMaxHeap(values, 8);
    long long buildComparisons = h.getComparisons();
    t.check(buildComparisons > 0, "building a heap counts comparisons");
    t.check(h.getSwaps() > 0, "building this heap needs swaps");

    h.resetCounters();
    t.check(h.getComparisons() == 0 && h.getSwaps() == 0, "resetCounters clears both counters");

    // Bottom-up construction is O(n), while inserting one at a time is
    // O(n log n). Ascending input is the case that shows it clearly: every
    // insert then has to sift the new value all the way up to the root.
    MaxHeap bottomUp;
    int many[64];
    for (int i = 0; i < 64; i++)
    {
        many[i] = i + 1;
    }
    bottomUp.resetCounters();
    bottomUp.buildMaxHeap(many, 64);

    MaxHeap oneByOne;
    oneByOne.resetCounters();
    for (int i = 0; i < 64; i++)
    {
        oneByOne.insert(many[i]);
    }
    t.check(bottomUp.isMaxHeap() && oneByOne.isMaxHeap(), "both construction routes give a heap");
    t.check(bottomUp.getSwaps() < oneByOne.getSwaps(),
            "bottom-up construction swaps fewer times than repeated insertion ("
            + TextUtil::intToText((int)bottomUp.getSwaps()) + " vs "
            + TextUtil::intToText((int)oneByOne.getSwaps()) + ")");
}

static void testLoadAndUndo(TestRunner& t)
{
    int values[6] = { 60, 50, 40, 30, 20, 10 };
    MaxHeap h;
    h.buildMaxHeap(values, 6);

    // the Act 4 undo path: remember the array, change it, load it back
    int snapshot[6];
    for (int i = 0; i < 6; i++)
    {
        snapshot[i] = h.get(i);
    }
    int savedSize = h.getHeapSize();

    int result = 0;
    h.extractMax(result);
    t.check(h.getHeapSize() == savedSize - 1, "extraction shrank the active heap");

    h.load(snapshot, 6, savedSize);
    bool restored = true;
    for (int i = 0; i < 6; i++)
    {
        if (h.get(i) != snapshot[i])
        {
            restored = false;
        }
    }
    t.check(restored && h.getHeapSize() == savedSize, "load restores the exact saved state");

    h.load(values, 6, 99);
    t.check(h.getHeapSize() == 6, "load clamps an oversized active heap size");
    h.load(values, 6, -3);
    t.check(h.getHeapSize() == 0, "load clamps a negative active heap size");
}

static void testValidatorCatchesBreakage(TestRunner& t)
{
    string error;

    int good[7] = { 90, 70, 80, 10, 20, 30, 40 };
    MaxHeap valid;
    valid.load(good, 7, 7);
    t.check(HeapValidator::checkHeap(valid, error), "validator accepts a correct Max Heap");
    t.check(error.length() == 0, "a passing check leaves the error message empty");

    int bad[7] = { 90, 70, 80, 10, 95, 30, 40 };
    MaxHeap broken;
    broken.load(bad, 7, 7);
    t.check(!HeapValidator::checkHeap(broken, error), "validator rejects a child above its parent");
    t.check(error.length() > 0, "a failing check explains itself: " + error);

    // the same 95 sitting just outside the active heap is a sorted value,
    // not a heap violation, so the same array now passes
    broken.load(bad, 5, 4);
    t.check(HeapValidator::checkHeap(broken, error),
            "values outside the active heap are not part of the heap property");

    int outOfOrder[5] = { 10, 5, 90, 20, 30 };
    MaxHeap region;
    region.load(outOfOrder, 5, 2);
    t.check(!HeapValidator::checkHeap(region, error), "validator rejects an unsorted sorted region");
}

// The deliverable asks for a long seeded random run with the validator after
// every single step, so a rare ordering bug cannot slip through.
static void testRandomOperations(TestRunner& t)
{
    Random rng(20261120);
    MaxHeap h;
    int start[12];
    for (int i = 0; i < 12; i++)
    {
        start[i] = rng.nextInt(1, 500);
    }
    h.buildMaxHeap(start, 12);

    bool alwaysValid = true;
    bool alwaysHeap = true;
    string error;
    int extractions = 0;
    int insertions = 0;

    for (int step = 0; step < 10000; step++)
    {
        int choice = rng.nextInt(0, 2);
        if (choice == 0 || h.getHeapSize() == 0)
        {
            h.insert(rng.nextInt(1, 500));
            insertions++;
        }
        else if (choice == 1)
        {
            int result = 0;
            int rootBefore = h.get(0);
            if (h.extractMax(result) && result == rootBefore)
            {
                extractions++;
            }
            else
            {
                alwaysHeap = false;
            }
            // put the sorted region back into play so the run keeps mixing
            int live[600];
            int liveCount = h.getHeapSize();
            for (int i = 0; i < liveCount; i++)
            {
                live[i] = h.get(i);
            }
            h.load(live, liveCount, liveCount);
        }
        else
        {
            h.heapify();
        }

        if (!HeapValidator::checkHeap(h, error))
        {
            alwaysValid = false;
        }
        if (!h.isMaxHeap())
        {
            alwaysHeap = false;
        }
        // keep the heap from growing without bound over 10,000 steps
        if (h.getHeapSize() > 500)
        {
            int live[600];
            for (int i = 0; i < 200; i++)
            {
                live[i] = h.get(i);
            }
            h.load(live, 200, 200);
            h.heapify();
        }
    }

    t.check(alwaysValid, "10,000 random heap operations: the validator passed after every step");
    t.check(alwaysHeap, "10,000 random heap operations: isMaxHeap held after every step");
    t.check(insertions > 1000 && extractions > 1000,
            "the random run exercised both inserts and extractions ("
            + TextUtil::intToText(insertions) + " inserts, "
            + TextUtil::intToText(extractions) + " extractions)");
}

void runHeapTests(TestRunner& t)
{
    t.line("");
    t.line("-- MaxHeap and Heap Sort (Act 4) --");
    testIndexMaths(t);
    testBuildMaxHeap(t);
    testSiftAndHeapify(t);
    testExtractMax(t);
    testPlayerSwap(t);
    testHeapSort(t);
    testCounters(t);
    testLoadAndUndo(t);
    testValidatorCatchesBreakage(t);
    testRandomOperations(t);
}

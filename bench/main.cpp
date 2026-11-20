// Benchmarks for spec section 10. Release build, EventLog = nullptr.
// Prints tables to the console and writes CSV files to bench/results/
// so the charts for the report can be made in Excel.
//
// Partner A: height comparison, search cost, rotation count, order statistics.
// Partner B: heap construction, Heap Sort, sorting comparison.
#include <iostream>
#include <fstream>
#include <SFML/System/Clock.hpp>
#include "model/BST.h"
#include "model/AVLTree.h"
#include "model/MaxHeap.h"
#include "core/Random.h"

using namespace std;

// median of 5 runs, our own tiny sort for 5 values
static double medianOf5(double v[5])
{
    for (int i = 1; i < 5; i++)
    {
        double value = v[i];
        int j = i - 1;
        while (j >= 0 && v[j] > value)
        {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = value;
    }
    return v[2];
}

// fills keys[0..n-1] with a seeded permutation of 1..n
static void makeRandomKeys(int keys[], int n, int seed)
{
    for (int i = 0; i < n; i++)
    {
        keys[i] = i + 1;
    }
    Random rng(seed);
    rng.shuffle(keys, n);
}

static void benchHeightAndSearch(ofstream& heightCsv, ofstream& searchCsv)
{
    cout << "\n== Height comparison and search cost ==\n";
    cout << "n      | input  | tree | height | avg cmp/search | time 10k searches (ms)\n";
    cout << "-------+--------+------+--------+----------------+-----------------------\n";
    heightCsv << "n,input,tree,height\n";
    searchCsv << "n,input,tree,avgComparisons,timeMs\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* keys = new int[10000];
    int* queries = new int[10000];

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        for (int input = 0; input < 2; input++)   // 0 sorted, 1 random
        {
            string inputName = input == 0 ? "sorted" : "random";
            if (input == 0)
            {
                for (int i = 0; i < n; i++)
                {
                    keys[i] = i + 1;
                }
            }
            else
            {
                makeRandomKeys(keys, n, 1000 + n);
            }
            Random queryRng(555);
            for (int i = 0; i < 10000; i++)
            {
                queries[i] = queryRng.nextInt(1, n);
            }

            for (int treeKind = 0; treeKind < 2; treeKind++)
            {
                string treeName = treeKind == 0 ? "BST" : "AVL";
                // BST* can hold either tree; AVLTree overrides insert/remove
                BST* tree = nullptr;
                if (treeKind == 0)
                {
                    tree = new BST();
                }
                else
                {
                    tree = new AVLTree();
                }
                for (int i = 0; i < n; i++)
                {
                    tree->insert(keys[i]);
                }

                double times[5];
                long long comparisons = 0;
                for (int run = 0; run < 5; run++)
                {
                    tree->resetCounters();
                    sf::Clock clock;
                    for (int q = 0; q < 10000; q++)
                    {
                        tree->search(queries[q]);
                    }
                    times[run] = (double)clock.getElapsedTime().asMicroseconds() / 1000.0;
                    comparisons = tree->getComparisons();
                }
                double ms = medianOf5(times);
                double avgCmp = (double)comparisons / 10000.0;

                cout.width(6);
                cout << n << " | ";
                cout.width(6);
                cout << inputName << " | ";
                cout.width(4);
                cout << treeName << " | ";
                cout.width(6);
                cout << tree->height() << " | ";
                cout.width(14);
                cout << avgCmp << " | " << ms << "\n";
                heightCsv << n << "," << inputName << "," << treeName << "," << tree->height() << "\n";
                searchCsv << n << "," << inputName << "," << treeName << "," << avgCmp << "," << ms << "\n";
                delete tree;
            }
        }
    }
    delete[] keys;
    delete[] queries;
}

static void benchRotations(ofstream& csv)
{
    cout << "\n== AVL rotation count ==\n";
    cout << "n      | input  | rot/insert | rot/delete | max rot in one delete\n";
    cout << "-------+--------+------------+------------+----------------------\n";
    csv << "n,input,rotPerInsert,rotPerDelete,maxRotPerDelete\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* keys = new int[10000];

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        for (int input = 0; input < 2; input++)
        {
            string inputName = input == 0 ? "sorted" : "random";
            if (input == 0)
            {
                for (int i = 0; i < n; i++)
                {
                    keys[i] = i + 1;
                }
            }
            else
            {
                makeRandomKeys(keys, n, 2000 + n);
            }

            AVLTree tree;
            tree.resetCounters();
            for (int i = 0; i < n; i++)
            {
                tree.insert(keys[i]);
            }
            double rotPerInsert = (double)tree.getRotations() / (double)n;

            long long beforeDeletes = tree.getRotations();
            int maxPerDelete = 0;
            Random deleteOrder(31337);
            int deleted = 0;
            for (int i = 0; i < n; i++)
            {
                int key = deleteOrder.nextInt(1, n);
                if (tree.remove(key))
                {
                    deleted++;
                    if (tree.getMaxRotationsLastOp() > maxPerDelete)
                    {
                        maxPerDelete = tree.getMaxRotationsLastOp();
                    }
                }
            }
            double rotPerDelete = deleted > 0
                ? (double)(tree.getRotations() - beforeDeletes) / (double)deleted : 0.0;

            cout.width(6);
            cout << n << " | ";
            cout.width(6);
            cout << inputName << " | ";
            cout.width(10);
            cout << rotPerInsert << " | ";
            cout.width(10);
            cout << rotPerDelete << " | " << maxPerDelete << "\n";
            csv << n << "," << inputName << "," << rotPerInsert << ","
                << rotPerDelete << "," << maxPerDelete << "\n";
        }
    }
    delete[] keys;
}

static void benchOrderStatistics(ofstream& csv)
{
    cout << "\n== Order statistics: kth on AVL vs inorder scan on BST ==\n";
    cout << "n      | AVL kth 1000 queries (ms) | BST scan 1000 queries (ms)\n";
    cout << "-------+---------------------------+---------------------------\n";
    csv << "n,avlKthMs,bstScanMs\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* keys = new int[10000];

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        makeRandomKeys(keys, n, 3000 + n);

        AVLTree avl;
        BST bst;
        for (int i = 0; i < n; i++)
        {
            avl.insert(keys[i]);
            bst.insert(keys[i]);
        }

        Random queryRng(777);
        int* ks = new int[1000];
        for (int i = 0; i < 1000; i++)
        {
            ks[i] = queryRng.nextInt(1, n);
        }

        double avlTimes[5];
        double bstTimes[5];
        for (int run = 0; run < 5; run++)
        {
            int result = 0;
            sf::Clock clock;
            for (int q = 0; q < 1000; q++)
            {
                avl.kth(ks[q], result);
            }
            avlTimes[run] = (double)clock.getElapsedTime().asMicroseconds() / 1000.0;

            sf::Clock clock2;
            for (int q = 0; q < 1000; q++)
            {
                bst.kthSmallest(ks[q], result);
            }
            bstTimes[run] = (double)clock2.getElapsedTime().asMicroseconds() / 1000.0;
        }
        double avlMs = medianOf5(avlTimes);
        double bstMs = medianOf5(bstTimes);

        cout.width(6);
        cout << n << " | ";
        cout.width(25);
        cout << avlMs << " | " << bstMs << "\n";
        csv << n << "," << avlMs << "," << bstMs << "\n";
        delete[] ks;
    }
    delete[] keys;
}

// ---------------------------------------------------------------------------
// Partner B benchmarks: heap construction, Heap Sort, sorting comparison.
// ---------------------------------------------------------------------------

static void swapInts(int& a, int& b)
{
    int hold = a;
    a = b;
    b = hold;
}

// The four input shapes the spec asks for in section 10.
static void makeArray(int a[], int n, int shape, int seed)
{
    Random rng(seed);
    for (int i = 0; i < n; i++)
    {
        if (shape == 0)
        {
            a[i] = rng.nextInt(1, 1000000);          // random
        }
        else if (shape == 1)
        {
            a[i] = i + 1;                            // already sorted
        }
        else if (shape == 2)
        {
            a[i] = n - i;                            // reverse sorted
        }
        else
        {
            a[i] = rng.nextInt(1, 10);               // duplicate heavy
        }
    }
}

static string shapeName(int shape)
{
    if (shape == 0)
    {
        return "random";
    }
    if (shape == 1)
    {
        return "sorted";
    }
    if (shape == 2)
    {
        return "reverse";
    }
    return "duplicates";
}

// Small inputs finish faster than the clock can measure, so the operation is
// repeated and the total divided. 200000 element-operations is enough to get
// a stable reading at every size used here.
static int repeatsFor(int n)
{
    int repeats = 200000 / n;
    if (repeats < 1)
    {
        repeats = 1;
    }
    return repeats;
}

static void insertionSort(const int in[], int n, int out[], long long& comparisons, long long& moves)
{
    for (int i = 0; i < n; i++)
    {
        out[i] = in[i];
    }
    for (int i = 1; i < n; i++)
    {
        int value = out[i];
        int j = i - 1;
        while (j >= 0)
        {
            comparisons++;
            if (out[j] <= value)
            {
                break;
            }
            out[j + 1] = out[j];
            moves++;
            j--;
        }
        out[j + 1] = value;
        moves++;
    }
}

static void selectionSort(const int in[], int n, int out[], long long& comparisons, long long& moves)
{
    for (int i = 0; i < n; i++)
    {
        out[i] = in[i];
    }
    for (int i = 0; i < n - 1; i++)
    {
        int smallest = i;
        for (int j = i + 1; j < n; j++)
        {
            comparisons++;
            if (out[j] < out[smallest])
            {
                smallest = j;
            }
        }
        if (smallest != i)
        {
            swapInts(out[i], out[smallest]);
            moves++;
        }
    }
}

static void mergeRange(int a[], int temp[], int lo, int mid, int hi,
                       long long& comparisons, long long& moves)
{
    int left = lo;
    int right = mid + 1;
    int out = lo;
    while (left <= mid && right <= hi)
    {
        comparisons++;
        if (a[left] <= a[right])
        {
            temp[out] = a[left];
            left++;
        }
        else
        {
            temp[out] = a[right];
            right++;
        }
        out++;
        moves++;
    }
    while (left <= mid)
    {
        temp[out] = a[left];
        left++;
        out++;
        moves++;
    }
    while (right <= hi)
    {
        temp[out] = a[right];
        right++;
        out++;
        moves++;
    }
    for (int i = lo; i <= hi; i++)
    {
        a[i] = temp[i];
    }
}

static void mergeSortRange(int a[], int temp[], int lo, int hi,
                           long long& comparisons, long long& moves)
{
    if (lo >= hi)
    {
        return;
    }
    int mid = lo + (hi - lo) / 2;
    mergeSortRange(a, temp, lo, mid, comparisons, moves);
    mergeSortRange(a, temp, mid + 1, hi, comparisons, moves);
    mergeRange(a, temp, lo, mid, hi, comparisons, moves);
}

// Median-of-three pivot, so sorted and reverse-sorted inputs do not push
// quick sort into its O(n^2) worst case and blow the stack.
static int choosePivot(int a[], int lo, int hi, long long& comparisons)
{
    int mid = lo + (hi - lo) / 2;
    comparisons += 3;
    if (a[mid] < a[lo])
    {
        swapInts(a[mid], a[lo]);
    }
    if (a[hi] < a[lo])
    {
        swapInts(a[hi], a[lo]);
    }
    if (a[hi] < a[mid])
    {
        swapInts(a[hi], a[mid]);
    }
    return a[mid];
}

static void quickSortRange(int a[], int lo, int hi, long long& comparisons, long long& moves)
{
    while (lo < hi)
    {
        int pivot = choosePivot(a, lo, hi, comparisons);
        int left = lo;
        int right = hi;
        while (left <= right)
        {
            while (true)
            {
                comparisons++;
                if (a[left] >= pivot)
                {
                    break;
                }
                left++;
            }
            while (true)
            {
                comparisons++;
                if (a[right] <= pivot)
                {
                    break;
                }
                right--;
            }
            if (left <= right)
            {
                swapInts(a[left], a[right]);
                moves++;
                left++;
                right--;
            }
        }
        // recurse into the smaller half and loop on the larger one, so the
        // recursion depth stays O(log n)
        if (right - lo < hi - left)
        {
            quickSortRange(a, lo, right, comparisons, moves);
            lo = left;
        }
        else
        {
            quickSortRange(a, left, hi, comparisons, moves);
            hi = right;
        }
    }
}

static void benchHeapConstruction(ofstream& csv)
{
    cout << "\n== Heap construction: bottom-up build vs n separate inserts ==\n";
    cout << "n      | input  | method     | time (ms) | comparisons | swaps\n";
    cout << "-------+--------+------------+-----------+-------------+----------\n";
    csv << "n,input,method,timeMs,comparisons,swaps\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* values = new int[10000];

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        int repeats = repeatsFor(n);
        for (int shape = 0; shape < 2; shape++)      // random and already ordered
        {
            makeArray(values, n, shape, 4000 + n);

            for (int method = 0; method < 2; method++)
            {
                string methodName = method == 0 ? "buildMaxHeap" : "n inserts";
                double times[5];
                for (int run = 0; run < 5; run++)
                {
                    sf::Clock clock;
                    for (int r = 0; r < repeats; r++)
                    {
                        MaxHeap heap;
                        if (method == 0)
                        {
                            heap.buildMaxHeap(values, n);
                        }
                        else
                        {
                            for (int i = 0; i < n; i++)
                            {
                                heap.insert(values[i]);
                            }
                        }
                    }
                    times[run] = (double)clock.getElapsedTime().asMicroseconds()
                               / 1000.0 / (double)repeats;
                }
                double ms = medianOf5(times);

                MaxHeap counted;
                counted.resetCounters();
                if (method == 0)
                {
                    counted.buildMaxHeap(values, n);
                }
                else
                {
                    for (int i = 0; i < n; i++)
                    {
                        counted.insert(values[i]);
                    }
                }

                cout.width(6);
                cout << n << " | ";
                cout.width(6);
                cout << shapeName(shape) << " | ";
                cout.width(10);
                cout << methodName << " | ";
                cout.width(9);
                cout << ms << " | ";
                cout.width(11);
                cout << counted.getComparisons() << " | " << counted.getSwaps() << "\n";
                csv << n << "," << shapeName(shape) << "," << methodName << "," << ms << ","
                    << counted.getComparisons() << "," << counted.getSwaps() << "\n";
            }
        }
    }
    delete[] values;
}

static void benchHeapSortExperiment(ofstream& csv)
{
    cout << "\n== Heap Sort ==\n";
    cout << "n      | input      | time (ms) | comparisons | swaps  | cmp / (n log2 n)\n";
    cout << "-------+------------+-----------+-------------+--------+------------------\n";
    csv << "n,input,timeMs,comparisons,swaps,comparisonsOverNLogN\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* values = new int[10000];
    int* sorted = new int[10000];

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        int repeats = repeatsFor(n);
        // log2(n) without <cmath>: count how often n halves before reaching 1
        double log2n = 0.0;
        int shrinking = n;
        while (shrinking > 1)
        {
            shrinking = shrinking / 2;
            log2n = log2n + 1.0;
        }

        for (int shape = 0; shape < 4; shape++)
        {
            makeArray(values, n, shape, 5000 + n);

            double times[5];
            for (int run = 0; run < 5; run++)
            {
                sf::Clock clock;
                for (int r = 0; r < repeats; r++)
                {
                    MaxHeap heap;
                    heap.heapSort(values, n, sorted);
                }
                times[run] = (double)clock.getElapsedTime().asMicroseconds()
                           / 1000.0 / (double)repeats;
            }
            double ms = medianOf5(times);

            MaxHeap counted;
            counted.resetCounters();
            counted.heapSort(values, n, sorted);
            double ratio = (double)counted.getComparisons() / ((double)n * log2n);

            cout.width(6);
            cout << n << " | ";
            cout.width(10);
            cout << shapeName(shape) << " | ";
            cout.width(9);
            cout << ms << " | ";
            cout.width(11);
            cout << counted.getComparisons() << " | ";
            cout.width(6);
            cout << counted.getSwaps() << " | " << ratio << "\n";
            csv << n << "," << shapeName(shape) << "," << ms << ","
                << counted.getComparisons() << "," << counted.getSwaps() << "," << ratio << "\n";
        }
    }
    delete[] values;
    delete[] sorted;
}

static void benchSortingComparison(ofstream& csv)
{
    cout << "\n== Heap Sort against the other sorting algorithms (random input) ==\n";
    cout << "n      | algorithm  | time (ms) | comparisons | moves    | extra space\n";
    cout << "-------+------------+-----------+-------------+----------+-------------\n";
    csv << "n,algorithm,timeMs,comparisons,moves,auxiliarySpace\n";

    int sizes[3] = { 100, 1000, 10000 };
    int* values = new int[10000];
    int* sorted = new int[10000];
    int* work = new int[10000];
    int* temp = new int[10000];

    string names[5] = { "heap", "insertion", "selection", "merge", "quick" };
    string space[5] = { "O(1)", "O(1)", "O(1)", "O(n)", "O(log n) stack" };

    for (int s = 0; s < 3; s++)
    {
        int n = sizes[s];
        int repeats = repeatsFor(n);
        makeArray(values, n, 0, 6000 + n);

        for (int algorithm = 0; algorithm < 5; algorithm++)
        {
            long long comparisons = 0;
            long long moves = 0;

            double times[5];
            for (int run = 0; run < 5; run++)
            {
                sf::Clock clock;
                for (int r = 0; r < repeats; r++)
                {
                    long long throwAwayComparisons = 0;
                    long long throwAwayMoves = 0;
                    if (algorithm == 0)
                    {
                        MaxHeap heap;
                        heap.heapSort(values, n, sorted);
                    }
                    else if (algorithm == 1)
                    {
                        insertionSort(values, n, sorted, throwAwayComparisons, throwAwayMoves);
                    }
                    else if (algorithm == 2)
                    {
                        selectionSort(values, n, sorted, throwAwayComparisons, throwAwayMoves);
                    }
                    else if (algorithm == 3)
                    {
                        for (int i = 0; i < n; i++)
                        {
                            work[i] = values[i];
                        }
                        mergeSortRange(work, temp, 0, n - 1, throwAwayComparisons, throwAwayMoves);
                    }
                    else
                    {
                        for (int i = 0; i < n; i++)
                        {
                            work[i] = values[i];
                        }
                        quickSortRange(work, 0, n - 1, throwAwayComparisons, throwAwayMoves);
                    }
                }
                times[run] = (double)clock.getElapsedTime().asMicroseconds()
                           / 1000.0 / (double)repeats;
            }
            double ms = medianOf5(times);

            if (algorithm == 0)
            {
                MaxHeap heap;
                heap.resetCounters();
                heap.heapSort(values, n, sorted);
                comparisons = heap.getComparisons();
                moves = heap.getSwaps();
            }
            else if (algorithm == 1)
            {
                insertionSort(values, n, sorted, comparisons, moves);
            }
            else if (algorithm == 2)
            {
                selectionSort(values, n, sorted, comparisons, moves);
            }
            else if (algorithm == 3)
            {
                for (int i = 0; i < n; i++)
                {
                    work[i] = values[i];
                }
                mergeSortRange(work, temp, 0, n - 1, comparisons, moves);
            }
            else
            {
                for (int i = 0; i < n; i++)
                {
                    work[i] = values[i];
                }
                quickSortRange(work, 0, n - 1, comparisons, moves);
            }

            cout.width(6);
            cout << n << " | ";
            cout.width(10);
            cout << names[algorithm] << " | ";
            cout.width(9);
            cout << ms << " | ";
            cout.width(11);
            cout << comparisons << " | ";
            cout.width(8);
            cout << moves << " | " << space[algorithm] << "\n";
            csv << n << "," << names[algorithm] << "," << ms << "," << comparisons << ","
                << moves << "," << space[algorithm] << "\n";
        }
    }
    delete[] values;
    delete[] sorted;
    delete[] work;
    delete[] temp;
}

int main()
{
    cout << "Vault Heist benchmarks. Events disabled, median of 5 runs.\n";

    ofstream heightCsv("bench/results/height_comparison.csv");
    ofstream searchCsv("bench/results/search_cost.csv");
    ofstream rotationCsv("bench/results/rotation_count.csv");
    ofstream orderCsv("bench/results/order_statistics.csv");
    ofstream heapBuildCsv("bench/results/heap_construction.csv");
    ofstream heapSortCsv("bench/results/heap_sort.csv");
    ofstream sortingCsv("bench/results/sorting_comparison.csv");

    benchHeightAndSearch(heightCsv, searchCsv);
    benchRotations(rotationCsv);
    benchOrderStatistics(orderCsv);
    benchHeapConstruction(heapBuildCsv);
    benchHeapSortExperiment(heapSortCsv);
    benchSortingComparison(sortingCsv);

    heightCsv.close();
    searchCsv.close();
    rotationCsv.close();
    orderCsv.close();
    heapBuildCsv.close();
    heapSortCsv.close();
    sortingCsv.close();
    cout << "\nCSV files written to bench/results/\n";
    return 0;
}

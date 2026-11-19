// Partner A benchmarks (spec section 10): height comparison, search cost,
// rotation count and order statistics. Release build, EventLog = nullptr.
// Prints tables to the console and writes CSV files to bench/results/
// so the charts for the report can be made in Excel.
#include <iostream>
#include <fstream>
#include <SFML/System/Clock.hpp>
#include "model/BST.h"
#include "model/AVLTree.h"
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

int main()
{
    cout << "Vault Heist benchmarks (Partner A). Events disabled, median of 5 runs.\n";

    ofstream heightCsv("bench/results/height_comparison.csv");
    ofstream searchCsv("bench/results/search_cost.csv");
    ofstream rotationCsv("bench/results/rotation_count.csv");
    ofstream orderCsv("bench/results/order_statistics.csv");

    benchHeightAndSearch(heightCsv, searchCsv);
    benchRotations(rotationCsv);
    benchOrderStatistics(orderCsv);

    heightCsv.close();
    searchCsv.close();
    rotationCsv.close();
    orderCsv.close();
    cout << "\nCSV files written to bench/results/\n";
    return 0;
}

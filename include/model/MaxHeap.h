#pragma once

// TODO(B): Partner B implements the Max Heap and Heap Sort (Act 4).
// Partner A only provides this compiling stub so the project links.

#include "core/EventLog.h"

class MaxHeap
{
public:
    MaxHeap();
    ~MaxHeap();
    int  parentOf(int i);                        // (i - 1) / 2
    int  leftOf(int i);                          // 2i + 1
    int  rightOf(int i);                         // 2i + 2
    void setEventLog(EventLog* eventLog);
    void buildMaxHeap(const int a[], int n);
    void siftDown(int i, int heapSize);
    void heapify();
    void siftUp(int i);
    bool extractMax(int& result);
    void insert(int value);
    void heapSort(const int a[], int n, int sortedOut[]);
    bool playerSwap(int i, int j);
    bool isMaxHeap();
    int  get(int i);
    int  totalCount();
    int  getHeapSize();
    void load(const int a[], int n, int heapSize);
    long long getComparisons();
    long long getSwaps();
    void resetCounters();

private:
    int* data;
    int capacity;
    int count;
    int heapSize;
    EventLog* log;
    long long comparisons;
    long long swaps;
};

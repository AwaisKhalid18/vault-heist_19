// TODO(B): Partner B replaces every stub body in this file.
#include "model/MaxHeap.h"

MaxHeap::MaxHeap()
{
    capacity = 256;
    data = new int[capacity];
    count = 0;
    heapSize = 0;
    log = nullptr;
    comparisons = 0;
    swaps = 0;
}

MaxHeap::~MaxHeap()
{
    delete[] data;
}

int MaxHeap::parentOf(int i) { return (i - 1) / 2; }
int MaxHeap::leftOf(int i) { return 2 * i + 1; }
int MaxHeap::rightOf(int i) { return 2 * i + 2; }

void MaxHeap::setEventLog(EventLog* eventLog) { log = eventLog; }
void MaxHeap::buildMaxHeap(const int a[], int n) {}
void MaxHeap::siftDown(int i, int heapSize) {}
void MaxHeap::heapify() {}
void MaxHeap::siftUp(int i) {}
bool MaxHeap::extractMax(int& result) { return false; }
void MaxHeap::insert(int value) {}
void MaxHeap::heapSort(const int a[], int n, int sortedOut[]) {}
bool MaxHeap::playerSwap(int i, int j) { return false; }
bool MaxHeap::isMaxHeap() { return false; }
int MaxHeap::get(int i) { return 0; }
int MaxHeap::totalCount() { return count; }
int MaxHeap::getHeapSize() { return heapSize; }
void MaxHeap::load(const int a[], int n, int heapSize) {}
long long MaxHeap::getComparisons() { return comparisons; }
long long MaxHeap::getSwaps() { return swaps; }
void MaxHeap::resetCounters() { comparisons = 0; swaps = 0; }

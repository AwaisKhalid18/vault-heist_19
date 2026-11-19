#pragma once

// Checks the Max Heap invariant: no child may beat its parent inside the
// active heap, and the sorted region beyond it must read in ascending order.

#include <string>
#include "model/MaxHeap.h"

using namespace std;

class HeapValidator
{
public:
    static bool checkHeap(MaxHeap& h, string& error);
};

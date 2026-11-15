#pragma once

// TODO(B): Partner B implements the heap invariant check.

#include <string>
#include "model/MaxHeap.h"

using namespace std;

class HeapValidator
{
public:
    static bool checkHeap(MaxHeap& h, string& error);
};

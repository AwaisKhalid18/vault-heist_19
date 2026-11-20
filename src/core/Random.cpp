#include "core/Random.h"

Random::Random(int seed)
{
    reseed(seed);
}

void Random::reseed(int seed)
{
    state = (unsigned int)seed;
    if (state == 0)
    {
        state = 1;
    }
}

unsigned int Random::nextRaw()
{
    // Numerical Recipes LCG constants
    state = state * 1664525u + 1013904223u;
    // the high bits are more random than the low bits
    return state >> 16;
}

int Random::nextInt(int lo, int hi)
{
    if (hi <= lo)
    {
        return lo;
    }
    int span = hi - lo + 1;
    return lo + (int)(nextRaw() % (unsigned int)span);
}

void Random::shuffle(int a[], int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = nextInt(0, i);
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

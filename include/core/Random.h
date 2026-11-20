#pragma once

// Our own small linear congruential generator.
// Every level stores a seed, so runs are reproducible and gradable.
// rand() and <random> are forbidden by the project rules.
class Random
{
public:
    Random(int seed);

    void reseed(int seed);
    int nextInt(int lo, int hi);        // uniform in [lo, hi], inclusive
    void shuffle(int a[], int n);       // Fisher-Yates shuffle

private:
    unsigned int state;

    unsigned int nextRaw();
};

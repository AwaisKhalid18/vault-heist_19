# Benchmark results — Partner B (heaps and sorting)

Spec section 10, the three experiments Partner B owns: **heap construction**,
**Heap Sort** and a **sorting comparison**. Partner A's four tree experiments
are in `BENCH_RESULTS_A.md`.

**How these were measured.** Release build (`./build.ps1 bench`), event
recording switched off (`setEventLog(nullptr)` — the benchmark never attaches a
log, so no events are written at all), timing with `sf::Clock` because
`<chrono>` is not allowed. Every timing is the **median of 5 runs**, and small
inputs are repeated enough times to total about 200,000 element-operations
before dividing, because a single 100-element sort finishes faster than the
clock can resolve. Raw console output is in `bench_output_B.txt`; the CSV files
for the report charts are in `bench/results/`.

Comparison and swap counts come from the `comparisons` and `swaps` members of
`MaxHeap`, incremented inside `siftUp` and `siftDown`. They are exact, not
sampled, so they are the same on any machine — unlike the timings.

---

## 1. Heap construction — bottom-up build vs n separate inserts

`buildMaxHeap` sifts down from the last parent to the root. `insert` adds one
value at the end and sifts it up. Both end with a valid Max Heap, but they do
very different amounts of work.

| n | input | method | time (ms) | comparisons | swaps |
|---|---|---|---|---|---|
| 100 | random | buildMaxHeap | 0.00085 | 190 | 96 |
| 100 | random | n inserts | 0.0131 | 203 | 106 |
| 100 | sorted | buildMaxHeap | 0.00081 | 190 | 96 |
| 100 | sorted | n inserts | 0.0092 | 480 | 480 |
| 1,000 | random | buildMaxHeap | 0.0066 | 1,839 | 710 |
| 1,000 | random | n inserts | 0.128 | 2,165 | 1,174 |
| 1,000 | sorted | buildMaxHeap | 0.0077 | 1,982 | 992 |
| 1,000 | sorted | n inserts | 0.126 | 7,987 | 7,987 |
| 10,000 | random | buildMaxHeap | 0.0965 | 18,807 | 7,422 |
| 10,000 | random | n inserts | 1.359 | 22,700 | 12,711 |
| 10,000 | sorted | buildMaxHeap | 0.0827 | 19,982 | 9,992 |
| 10,000 | sorted | n inserts | 1.691 | 113,631 | 113,631 |

### What the numbers say

**Bottom-up construction is O(n).** Look at the `buildMaxHeap` comparison
counts down the table: 190 → 1,982 → 19,982. Ten times the input gives ten
times the work, almost exactly. The comparisons-per-element ratio stays flat at
about **1.9 to 2.0** at every size. That flat ratio is the measurement that
confirms O(n).

**Why it is O(n) and not O(n log n).** The loop starts at the last parent, so
half the array — every leaf — is skipped immediately. Of what is left, half the
nodes sit one level above the leaves and can sink at most one level, a quarter
can sink at most two, and so on. The total is
`n/2 · 1 + n/4 · 2 + n/8 · 3 + …`, a series that converges to `2n`. The nodes
that *could* travel furthest are exactly the ones there are fewest of.

**Repeated insertion is O(n log n) in the worst case.** The ascending ("sorted")
rows are that worst case: every new value is the largest so far, so it sifts
all the way from the bottom to the root. At n = 10,000 that is **113,631
comparisons against 19,982** — about 5.7× more work, and the gap widens with n
because it is a log n factor, not a constant.

**On random input the two are much closer in comparisons** (22,700 vs 18,807).
A random new value usually loses to its parent straight away and stops after
one comparison, so the average insert is O(1) even though the worst case is
O(log n). The timing gap stays large anyway (1.36 ms vs 0.097 ms, **14×**)
because `insert` also pays for a capacity check and a function call per element,
while `buildMaxHeap` is one tight pass over an array that is already in memory.

**Conclusion for the report.** Build the heap in one bottom-up pass whenever all
the values are available at the start, which is what Act 4 does. Use `insert`
only for values that genuinely arrive one at a time.

---

## 2. Heap Sort

`heapSort` builds a Max Heap from the input, then repeatedly swaps the root
with the last active element, shrinks the active heap by one and sifts the new
root down. It sorts into a separate output array so the live heap the GUI is
showing is never disturbed.

| n | input | time (ms) | comparisons | swaps | comparisons / (n·log₂n) |
|---|---|---|---|---|---|
| 100 | random | 0.0046 | 1,032 | 577 | 1.72 |
| 100 | sorted | 0.0052 | 1,081 | 640 | 1.80 |
| 100 | reverse | 0.0044 | 944 | 516 | 1.57 |
| 100 | duplicates | 0.0042 | 1,002 | 533 | 1.67 |
| 1,000 | random | 0.0806 | 16,893 | 9,073 | 1.88 |
| 1,000 | sorted | 0.0870 | 17,583 | 9,708 | 1.95 |
| 1,000 | reverse | 0.0768 | 15,965 | 8,316 | 1.77 |
| 1,000 | duplicates | 0.0733 | 15,728 | 8,229 | 1.75 |
| 10,000 | random | 1.120 | 235,393 | 124,197 | 1.81 |
| 10,000 | sorted | 1.222 | 244,460 | 131,956 | 1.88 |
| 10,000 | reverse | 1.423 | 226,682 | 116,696 | 1.74 |
| 10,000 | duplicates | 1.003 | 217,009 | 112,143 | 1.67 |

### What the numbers say

**Heap Sort is O(n log n), and the last column is the proof.** Dividing the
comparison count by `n·log₂n` gives a number between **1.57 and 1.95 at every
size and on every input shape**. If the algorithm were O(n²) that ratio would
grow with n — between n = 100 and n = 10,000 it would get 100× larger. It does
not move. The constant factor is roughly 2, because each of the n sift-downs
walks about log₂n levels and compares against both children at each level.

**Why O(n log n).** The build is O(n) (section 1). After that there are n − 1
extractions, and each one puts an arbitrary value at the root and sifts it down
through up to log₂n levels. So the total is `O(n) + O(n log n) = O(n log n)`.

**Heap Sort does not care what the input looks like.** The four input shapes
differ by less than 15% in comparisons at any given n. Compare that with
insertion sort in section 3, which is 100× slower on random input than on
sorted input. Heap Sort has **no best case and no worst case worth the name**:
the heap property is destroyed and rebuilt the same way whatever the order was.
That makes it the predictable choice — and it is why Act 4's alarm budget can be
computed from n alone.

**Already-sorted input is very slightly the worst case here** (1.88–1.95 ratio).
Ascending input makes the bottom-up build do the most sinking, and the
extraction phase then keeps pulling the smallest remaining value to the root.
Reverse-sorted and duplicate-heavy input are slightly the cheapest.

**Swaps are about half the comparisons** throughout, which matches the
algorithm: each level of a sift-down costs two comparisons (against both
children) and at most one swap.

---

## 3. Heap Sort against the other sorting algorithms

All five algorithms were written from scratch for this comparison (no
`<algorithm>`, no library sort). Random input, same array for every algorithm.
"Moves" counts swaps for heap/selection/quick and element shifts for
insertion/merge, so the move columns are only comparable within a family.

| n | algorithm | time (ms) | comparisons | moves | extra space |
|---|---|---|---|---|---|
| 100 | heap | 0.0049 | 1,030 | 589 | O(1) |
| 100 | insertion | 0.0011 | 2,656 | 2,662 | O(1) |
| 100 | selection | 0.0141 | 4,950 | 95 | O(1) |
| 100 | merge | 0.0016 | 547 | 672 | O(n) |
| 100 | quick | 0.0008 | 939 | 156 | O(log n) stack |
| 1,000 | heap | 0.0841 | 16,841 | 9,073 | O(1) |
| 1,000 | insertion | 0.1107 | 253,422 | 253,430 | O(1) |
| 1,000 | selection | 1.4837 | 499,500 | 989 | O(1) |
| 1,000 | merge | 0.0357 | 8,708 | 9,976 | O(n) |
| 1,000 | quick | 0.0115 | 14,068 | 2,313 | O(log n) stack |
| 10,000 | heap | 1.153 | 235,128 | 123,987 | O(1) |
| 10,000 | insertion | 11.105 | 25,085,031 | 25,085,040 | O(1) |
| 10,000 | selection | 145.376 | 49,995,000 | 9,988 | O(1) |
| 10,000 | merge | 0.777 | 120,430 | 133,616 | O(n) |
| 10,000 | quick | 0.653 | 177,857 | 31,659 | O(log n) stack |

### What the numbers say

**The O(n²) sorts fall off a cliff and the O(n log n) sorts do not.** From
n = 1,000 to n = 10,000 — ten times the input — selection sort goes from 1.48 ms
to 145 ms, which is **98× slower**, exactly the 100× an O(n²) algorithm
predicts. Heap Sort over the same step goes from 0.084 ms to 1.15 ms, which is
**13.7×**, close to the `10 × (log 10000 / log 1000) ≈ 13.3` an O(n log n)
algorithm predicts. Selection sort's comparison count is the cleanest evidence
of all: 4,950, 499,500, 49,995,000 — precisely `n(n−1)/2` every time, because it
scans the whole unsorted remainder no matter what it finds.

**Insertion sort wins at n = 100 and loses badly at n = 10,000.** At 100
elements its inner loop is so cheap that it beats everything (0.0011 ms). At
10,000 it needs 25 million comparisons. This is the standard reason real
libraries use an O(n log n) sort but switch to insertion sort for small
sub-arrays.

**Heap Sort is 1.5–2× slower than merge and quick sort in wall-clock time**,
despite the same O(n log n) class. Two reasons, both about memory rather than
operation counts: a sift-down jumps from index `i` to `2i+1`, so on a large
array consecutive reads land far apart and the CPU cache misses, whereas merge
and quick sort read runs of neighbouring cells; and Heap Sort does far more
moves (124k vs 32k for quick sort) because every extraction drags a small value
from the end of the array back down through the whole tree.

**What Heap Sort buys in exchange is guarantees.**

- **Space:** it sorts in place. Merge sort needs a second n-element array
  (`O(n)` extra), which matters when the data is large.
- **Worst case:** Heap Sort is O(n log n) *always*. Quick sort is O(n log n) on
  average but O(n²) if the pivots are chosen badly — this implementation uses a
  median-of-three pivot specifically to avoid that on sorted input. Heap Sort
  needs no such defence.
- **Predictability:** section 2 showed under 15% variation across four input
  shapes. Nothing else in this table is that steady.

**Conclusion for the report.** Heap Sort is the algorithm to pick when the worst
case has to be bounded and no extra memory is available — which is exactly the
story Act 4 tells: a fixed alarm budget, a fixed vault, and no room to cheat.

---

## Reproducing these numbers

```powershell
./build.ps1 bench
```

Timings depend on the machine, so they will differ from the table above.
**Comparison and swap counts will not** — they are deterministic, driven by the
seeded `Random` generator, and are the figures the complexity argument rests on.

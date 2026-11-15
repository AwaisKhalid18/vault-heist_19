# Benchmark results — Partner A (trees)

Method: Release build, events disabled (`setEventLog(nullptr)`), fixed
seeds, timings with `sf::Clock`, **median of 5 runs**. Raw data for the
Excel charts: `bench/results/*.csv` (regenerate with `./build.ps1 bench`).
Machine: Windows 11, MSVC x64. Numbers below are one real run; reruns
vary by a few percent but the shapes never change.

## 1. Height comparison

| n | input | BST height | AVL height |
|---|---|---|---|
| 100 | sorted | **100** | 7 |
| 100 | random | 16 | 8 |
| 1,000 | sorted | **1,000** | 10 |
| 1,000 | random | 20 | 12 |
| 10,000 | sorted | **10,000** | 14 |
| 10,000 | random | 30 | 16 |

**Why:** a BST's shape is its insertion order. Sorted keys always go
right, so the tree is a chain of height n — the spec's boss level 2.5.
Random order gives ~1.39·log2(n) on average (log2(10,000) ≈ 13.3, and we
see 30 — the expected constant-factor overhead). The AVL invariant
|BF| ≤ 1 forces height ≤ 1.44·log2(n+2) **no matter the order**: 14
levels for 10,000 sorted keys.

## 2. Search cost (10,000 random searches)

| n | input | tree | avg comparisons | time (ms) |
|---|---|---|---|---|
| 100 | sorted | BST | 50.7 | 3.3 |
| 100 | sorted | AVL | 5.8 | 1.0 |
| 1,000 | sorted | BST | 504.7 | 30.0 |
| 1,000 | sorted | AVL | 9.0 | 1.5 |
| 10,000 | sorted | BST | **4,822.8** | **284.4** |
| 10,000 | sorted | AVL | **12.4** | **2.3** |
| 10,000 | random | BST | 16.3 | 2.6 |
| 10,000 | random | AVL | 12.5 | 2.5 |

**Why:** search cost is the depth of the key. In the sorted-input chain
the average key sits at depth n/2 → ~4,823 comparisons, O(n) per search.
The AVL needs ~log2(n) ≈ 12-14 → the same 10,000 searches run **~120×
faster**. On random input the BST is already close to balanced, so the
two trees are within ~30% of each other — balancing only costs a little
when it is not needed, and saves everything when it is.

## 3. AVL rotation count

| n | input | rotations / insert | rotations / delete | max in one delete |
|---|---|---|---|---|
| 100 | sorted | 0.93 | 0.12 | 2 |
| 100 | random | 0.81 | 0.29 | 3 |
| 1,000 | sorted | 0.99 | 0.23 | 4 |
| 1,000 | random | 0.68 | 0.32 | 4 |
| 10,000 | sorted | 1.00 | 0.21 | 4 |
| 10,000 | random | 0.70 | 0.38 | 6 |

**Why:** balancing is cheap. An insert needs **at most one** repair
(single or double); a rising sorted sequence triggers one almost every
time (→ ~1.0), random input only ~0.7. A delete can cascade repairs up
the path, but even then it averages well under one rotation and the
worst single delete used 6 — consistent with the O(log n) bound (the
tree is ~16 levels tall).

## 4. Order statistics: kth on AVL vs inorder scan on BST (1,000 queries)

| n | AVL size-guided kth (ms) | BST inorder scan (ms) |
|---|---|---|
| 100 | 0.32 | 0.65 |
| 1,000 | 0.29 | 2.77 |
| 10,000 | 0.49 | **59.60** |

**Why:** the plain BST answers "k-th smallest" by walking the inorder
sequence and counting — O(n) per query, so the time grows linearly
(10× n → ~20× time here, because the tree also got deeper). The AVL
stores subtree sizes and descends: left size `ls` tells whether the
answer is left (k ≤ ls), here (k = ls+1) or right (continue with
k−ls−1) — O(log n) per query. At n = 10,000 that is a **~120×** speedup,
and the gap keeps widening with n.

## Takeaway for the report

O(n) versus O(log n) is not an abstract label: with only 10,000 keys the
unbalanced structure is already two orders of magnitude slower, and the
"cost" of AVL maintenance is at most one rotation per insert. Height is
the single number that controls everything a tree does.

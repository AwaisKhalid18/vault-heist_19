# Report sections — Partner A (trees)

Material for the written report: architecture of the tree side,
complexity analysis and the BST vs AVL benchmark discussion.
(Partner B merges this with the heap sections into the final report.)

## 1. Architecture (tree side)

The guiding rule is **the data structures know nothing about the GUI**.

```
BST / AVLTree  --writes-->  EventLog (events + preorder snapshots)
                                 |
                                 v
                    AnimationController (timing, step, scrub, speed)
                                 |
                                 v
         LayoutEngine (inorder x, depth y)  ->  TreeRenderer (SFML)
```

- `src/model` is pure C++ (no SFML include anywhere) and is exercised by
  `run_tests` without a window.
- Every operation appends typed `Event`s; structural events carry a
  snapshot of the whole tree shape. The GUI slides nodes **by id**
  between snapshots, so one animation mechanism covers insert, delete,
  the successor value swap and all four rotations.
- Benchmarks pass `nullptr` as the event log, so measurement runs pay
  zero event cost (the spec's "flag to disable events").
- Levels compute `optimalSteps` by running the model silently
  (`searchPathLength`, simulated deletes, pruned range walks), never
  from numbers typed into JSON.

## 2. Complexity analysis

### BST
A BST of height h does search / insert / delete in **O(h)**: each step
discards one subtree. h depends entirely on insertion order:
- random order → h ≈ 1.39·log2(n) on average;
- sorted order → every insert goes right, h = n (a linked list in
  disguise). This degeneration is the entire plot of level 2.5.

Range query with pruning costs O(h + m) for m reported keys: the walk
is the two boundary paths plus the in-range nodes. `kthSmallest` on a
plain BST is an O(n) inorder scan.

### AVL
The AVL invariant |BF| ≤ 1 at every node forces at least Fibonacci-many
nodes for a given height, giving **h ≤ 1.44·log2(n+2)**: all path
operations become guaranteed O(log n). Insert repairs with at most one
single/double rotation; delete may rotate once per level, still
O(log n). Each rotation is O(1) pointer surgery plus the O(h) upward
height/size fix that insert/delete do anyway.

The `size` augmentation (subtree node counts, maintained by the same
upward loop) turns order statistics into descents: `kth` and `rank` in
O(log n), `countInRange(lo,hi) = rank(hi) − rank(lo−1)`.

## 3. Benchmark discussion (numbers in docs/BENCH_RESULTS_A.md)

- **Height comparison.** Random input: both trees stay around log2(n);
  AVL slightly lower and deterministic. Sorted input: the BST hits
  height = n exactly (10,000 for n = 10,000) while the AVL stays at
  ~log2(n) — the clearest picture of why balancing exists.
- **Search cost.** Average comparisons track the height: ~n/2 per search
  in the sorted BST versus ~log2(n) in the AVL. At n = 10,000 that is
  thousands of comparisons versus about 13 — a factor of several hundred
  in time.
- **Rotation count.** Rotations are cheap and rare: ~0.7 per random
  insert, 1.0 per sorted insert (every insert of a rising sequence
  triggers exactly one rebalance), well under one per delete on average;
  the maximum per single delete stays small (O(log n) bound).
- **Order statistics.** 1,000 `kth` queries: the AVL's size-guided
  descent is O(log n) each, the plain BST's inorder counter is O(n)
  each; at n = 10,000 the scan is slower by orders of magnitude.

Method: fixed seeds, events disabled, Release build, each timing is the
median of 5 runs (`sf::Clock`); raw tables land in `bench/results/*.csv`
for the Excel charts.

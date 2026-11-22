# Test report

`./build.ps1 test` builds and runs `run_tests` (our own `TestRunner`, no
framework) and saves the raw output to `docs/test_output_A.txt`.

**Current result: 366 passed, 0 failed.**

The runner links only `vh_model`, so the whole suite runs with no window
open. That is the separation rule enforced by the build rather than by
convention: if a data structure ever reached for SFML, these tests would
stop linking.

| Suite | Area | Checks |
|---|---|---|
| `test_core.cpp` | Events, log, stack/queue, random, text, scoring, loader | 24 |
| `test_bst.cpp` | Search, insert, three delete cases, ranges, degeneration | 55 |
| `test_avl.cpp` | Balance factors, four rotations, rebalancing, order statistics | 38 |
| `test_tree_validator.cpp` | Height/size/ordering invariants and their violation | 10 |
| `test_tree_events.cpp` | Event emission and snapshot correctness | 14 |
| `test_levels.cpp` | Acts 2–3 winnability and optimal step counts | 32 |
| `test_binarytree.cpp` | Traversals, reconstruction, height, diameter, LCA, mirror | 53 |
| `test_heap.cpp` | Build, sift, heapify, extract, heap sort, counters, swaps | 76 |
| `test_levels_b.cpp` | Acts 1 and 4 winnability and optimal step counts | 18 |
| `test_save.cpp` | Serialisation and reload of trees, heaps, progress, settings | 46 |
| **Total** | | **366** |

Suites 1–6 are Partner A's (trees); suites 7–10 are Partner B's (general
binary trees, heaps and the save system).

## What each suite checks

### Core (`tests/test_core.cpp`)
- `NodeStack` / `NodeQueue` grow past their initial capacity and keep
  LIFO / FIFO order; popping an empty stack returns nullptr.
- `Random`: same seed → same sequence, values stay inside [lo, hi],
  shuffle is a permutation (keeps every element).
- `TextUtil`: int→text and text→int round trips, broken input rejected.
- `ScoreSystem`: capacity formula ceil(1.5·opt)+2 for even and odd
  optimal; all four star outcomes; the hint star-cap; the score formula
  and its floor at 0.
- `LevelLoader`: parses a real level file (strings, ints, arrays,
  hints) and reads the 10-line campaign list, skipping comments.

### BST (`tests/test_bst.cpp`)
- Insert/search hit and miss; size and height (leaf = 1) after inserts.
- Duplicates rejected (root and inner node), size unchanged.
- Delete: leaf, one LEFT child, one RIGHT child, two children (successor
  swap verified), deleting the root, deleting a missing key — each
  followed by a full validator pass.
- `deleteCaseOf` for every case including NOT_FOUND.
- findMin/findMax, and both failing on an empty tree.
- successor/predecessor, including for a key not in the tree and the
  "no successor of the maximum" / "no predecessor of the minimum" cases.
- rangeQuery: correct sorted contents, empty range, and real pruning
  (the query [75,85] on the 7-node demo tree enters only 3 rooms,
  counted from VISIT events).
- kthSmallest (first, middle, last, out of range), lcaBST (normal,
  ancestor case, missing key), searchPathLength, inorderKeys sorted.
- Deep-path safety: 10,000 **sorted** inserts give height exactly
  10,000, search still works, clear() frees it — all loops, no crash.
- copyFrom keeps node ids AND nextId (new nodes get identical ids in
  the copy and the original).

### AVL (`tests/test_avl.cpp`)
- One insertion test per rotation case (LL, RR, LR, RL): the ROTATE
  event carries the right case constant, the expected key becomes the
  root, the validator passes.
- Deletion rebalancing, including the **balance factor 0 on the heavy
  child** case (single rotation) and a delete forcing a double repair.
- caseAt on a manual violation; predictInsert foresees LL/LR/none and
  provably changes nothing.
- Manual mode: rotateRightAt repairs a chain, impossible rotations are
  refused, rotations never change the inorder sequence (checked by
  rotating left then right and comparing).
- A seeded **2,000-operation** random insert/delete run where after
  EVERY operation: the validator passes (heights, sizes, BF, parents),
  size matches a reference sorted array (own insertion sort), and three
  random kth and rank queries match the array.
- countInRange against the reference.
- A seeded **10,000-operation** stress run with the validator after
  every step, plus a final O(log n) height check.

### Events (`tests/test_tree_events.cpp`)
- search emits exactly one VISIT + one COMPARE per room, equal to the
  comparison counter; COMPARE events carry the chosen side; every batch
  ends with DONE.
- A nullptr log records nothing (and nothing crashes).
- INSERT / ROTATE / UNLINK events carry snapshots, and each snapshot is
  compared node-by-node (ids, keys, links, heights, sizes) with the
  live tree.
- BALANCE_ALERT precedes its ROTATE; two-children delete emits
  SWAP_VALUE before UNLINK; saveArray/getArray round trip.

### Validator (`tests/test_tree_validator.cpp`)
- A healthy tree passes all three checks; then each invariant is broken
  on purpose and must be caught: wrong ordering, wrong stored height,
  wrong stored size, bad parent pointer, a cycle, and a BF violation
  that only checkAVL flags.

### Levels (`tests/test_levels.cpp`)
- All 10 Act 2-3 JSON files load.
- 2.1: the built tree is valid, the target is ≥3 rooms deep, perfect
  play fits the capacity, and a full alarm scores 0 stars.
- 2.2: the three seeded inserts work and fit the capacity.
- 2.3: the crafted tree offers all three delete cases and perfect
  deletions fit the capacity.
- 2.4: the range holds ≥3 keys and the pruned walk (7 rooms) beats the
  full tree (14 rooms).
- 2.5: sorted inserts give height = n and the boss is winnable under
  the shrinking capacity.
- 3.1: exactly 2 unbalanced rooms. 3.2: LL ≥ 2 and RR ≥ 2 across the
  repair waves. 3.3: LR ≥ 2 and RL ≥ 2. 3.4: the prediction script
  contains all four cases; across Act 3 every rotation type appears at
  least twice (spec rule). 3.5: the k-th most valuable treasure exists,
  the descent is within the tree height and the capacity.

### BinaryTree (`tests/test_binarytree.cpp`)
- All four traversals checked against a hand-worked 7-room example, and
  every iterative version compared against its recursive twin (the
  iterative ones use our own `NodeStack` / `NodeQueue`, not the STL).
- Empty tree: every traversal writes nothing, height 0, diameter 0,
  counts as symmetric, has no root. Single node: height 1 (leaf = 1),
  diameter 1.
- `maxCount` is respected by every traversal, so a caller's buffer is
  never overrun.
- Reconstruction both ways: preorder + inorder and postorder + inorder
  rebuild the same tree, the round trip is stable, it keeps the height,
  rebuilding replaces the previous tree, and 0 rooms leaves it empty.
- Height, diameter and level order on the example tree; LCA for
  siblings, for distant rooms, for a room and its own child, and the
  false result when one or both ids are missing.
- `pathTo` for a deep room, for the root, and for a missing room.
- Mirroring: it changes the tree, reverses the inorder sequence, keeps
  the height, is its own inverse, and a mirror-image tree reports
  itself symmetric.
- **300 random shapes**, where for every one: recursive and iterative
  traversals agree, every room has a valid root path,
  height <= diameter <= room count, and mirroring twice is the identity.

### MaxHeap and Heap Sort (`tests/test_heap.cpp`)
- Index arithmetic first: children of the root, children of index 3,
  shared parents, and `parentOf` undoing `leftOf` / `rightOf`.
- Construction from empty, single, random, already-descending,
  reverse-sorted, all-duplicate and negative input; the maximum always
  reaches the root and the validator accepts the result. An already
  valid heap is built with **zero swaps**, which is the linear-time
  bottom-up build visible in a test.
- `siftDown` repairs a disturbed root and lifts the larger child;
  `heapify` repairs a scrambled array and brings the maximum up.
- Insert: a new maximum sifts to the root, both counts grow, the heap
  stays valid, the array grows past its first capacity, and **400
  inserts** still leave a valid Max Heap.
- Extraction: refused on an empty heap, always returns the current
  root, leaves a valid heap each time, comes out in descending order,
  and the extracted values remain in the array as the sorted region —
  so repeated extraction leaves the array sorted ascending.
- `playerSwap` (the move Act 4 gives the player) accepts a parent-child
  exchange in either argument order and rejects siblings, grandchildren,
  a cell swapped with itself, negative indices, indices past the heap,
  and any cell outside the active heap.
- Heap sort on empty, single, random, sorted, reverse-sorted,
  duplicate-heavy and negative input, each against a reference sort; it
  leaves the live heap and its active size untouched; and **200 random
  arrays** always match the reference.
- The comparison and swap counters used by the benchmarks are checked,
  since Section 10 of the report depends on them.

### Levels, Acts 1 and 4 (`tests/test_levels_b.cpp`)
- Every Partner B level file loads with an id, type, briefing and hints,
  and the campaign lists all 5 Act 1 and all 5 Act 4 levels.
- Each Act 1 level builds a tree of the size its file asks for, holds
  between 6 and 15 rooms as the spec requires, and its preorder +
  inorder rebuilds its own tree. Room values are distinct, so that
  reconstruction is unique.
- Each Act 4 level builds an array of the right size, can be finished
  inside its own alarm capacity, and leaves a valid heap after perfect
  play.
- The generators are checked against their own promises: `heap`
  produces a valid Max Heap, `disturbed` really does break it and the
  break is always repairable, descending input is already a heap, and
  `sorted` produces ascending input.
- The same seed rebuilds exactly the same vault in both acts, which is
  what makes a level reproducible for marking.

### SaveManager (`tests/test_save.cpp`)
- Progress: a fresh save has no stars, an unplayed level is not cleared,
  a finished level records its stars and score, a worse replay does not
  lower the recorded best while a better one raises it, totals add up,
  and `clear()` wipes everything.
- Leaderboard: starts empty, records runs, stays sorted highest first,
  never grows past its limit, keeps the best run on top, and drops a run
  too slow for the table.
- File round trip: level progress, per-level stars, the leaderboard, the
  animation speed, the validator and tooltip settings and the endless
  seed all survive a save and load. A missing save file fails cleanly
  instead of crashing and leaves an empty save.
- Tree serialisation: preorder with null markers (`50 30 # # 70 60 # # #`),
  an empty tree as a single marker, a leaf with both children missing,
  and **200 random vaults** that round trip unchanged. Truncated text,
  rubbish and trailing rubbish are all rejected.
- Heap serialisation: the array plus the active size, a half-sorted heap
  (so both the active heap and the sorted region survive), an empty
  heap, and rubbish rejected.

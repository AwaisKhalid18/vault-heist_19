# Test report — Partner A

`./build.ps1 test` builds and runs `run_tests` (our own `TestRunner`, no
framework) and saves the raw output to `docs/test_output_A.txt`.

**Current result: 173 passed, 0 failed.**

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

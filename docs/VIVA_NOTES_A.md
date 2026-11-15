# Viva notes — Partner A (BST, AVL, events, undo)

Simple explanations, worked examples, complexities and likely questions
for every algorithm Partner A owns. All examples use this tree:

```
            50
         /      \
       30        70
      /  \      /  \
    20    40  60    80
```

---

## 1. BST search

**Idea.** Start at the root. If the key equals the room's key, done. If it
is smaller, go left; if larger, go right. The ordering guarantees the key
can only live on that one path.

**Worked example: search 40.** 40 < 50 → left. 40 > 30 → right. 40 = 40 →
found. 3 rooms visited, 3 comparisons.

**Complexity.** O(h) time, O(1) space (h = height). Balanced: O(log n);
degenerate chain: O(n).

**Why loops, not recursion?** A BST built from sorted keys has height n
(10,000 in our benchmark). 10,000 nested calls overflow the Windows
stack, so every BST walk in our code is a `while` loop.

**Likely questions.**
1. *Worst case of search?* O(n), when the tree is a chain (sorted inserts).
2. *Best shape?* Balanced, height ≈ log2(n).
3. *How many comparisons for a hit at depth d?* d (one per room, our
   convention counts the equality check at each room as one comparison).
4. *Why no duplicates?* The ordering rule "smaller left, larger right"
   leaves no place for an equal key; we reject them and show a message.
5. *What does search emit for the GUI?* One VISIT and one COMPARE event
   per room, then DONE; the GUI animates these, it never re-runs logic.

## 2. BST insert

**Idea.** Walk exactly like search. When the next pointer is null, the
new node is linked there. Then walk UP with a loop fixing `height` and
`size` of the ancestors.

**Worked example: insert 45.** 45 < 50 left, 45 > 30 right, 45 > 40
right → 40's right is null → new room 45 becomes 40's right child.
Heights on the path 40, 30 update.

**Complexity.** O(h) time, O(1) space.

**Likely questions.**
1. *Where does a new key always land?* At a leaf position (end of its
   search path).
2. *What changes above it?* Only height/size of its ancestors.
3. *What if the key exists?* Return false, emit the compare events and a
   "duplicate rejected" DONE — the GUI shows the message.
4. *Cost of the upward fix?* O(h), one visit per ancestor.
5. *Why do ids matter?* The GUI tracks rooms by id so animations follow
   the same node across moves.

## 3. BST delete (three cases)

**Idea.** Find the node, then:
- **Leaf:** unlink it from its parent.
- **One child:** the parent adopts that child (bypass).
- **Two children:** find the **inorder successor** (smallest key of the
  right subtree = go right once, then left to the end), swap the values,
  then delete the successor node — which now has at most one child.

**Worked example: delete 50 (two children).** Successor of 50 = min of
right subtree = 60. Swap values (50 ↔ 60), then delete the node that now
holds 50 (old 60's position, a leaf). Result: root shows 60.

**Why the successor has no left child:** we walked left as far as
possible — one more left child would contradict "as far as possible".

**Complexity.** O(h) time, O(1) space.

**Likely questions.**
1. *Why swap values instead of relinking the node?* Fewer pointer
   changes; the node ids stay put, so the GUI animates a value swap.
2. *Could we use the predecessor instead?* Yes (largest of the left
   subtree); either is correct, we follow the spec's successor.
3. *Which case is deleting the root of our demo tree?* Two children.
4. *What if the key is missing?* Walk ends in null → return false.
5. *What events are emitted?* VISIT/COMPARE down, SWAP_VALUE (two-child
   case), UNLINK, height/size updates, DONE — each with a snapshot so
   the GUI can slide rooms to their new layout.

## 4. Successor / predecessor (top-down)

**Idea.** `successor(k)`: walk from the root keeping the **best
candidate**: whenever the room's key is bigger than k, remember it and go
left (maybe something smaller still beats it); otherwise go right. The
last remembered room is the smallest key > k.

**Worked example: successor(45).** 50 > 45 → candidate 50, left. 30 ≤ 45
→ right. 40 ≤ 45 → right → null. Answer 50. (Works although 45 itself is
not in the tree.)

**Complexity.** O(h), O(1) space. Fails (returns false) only when no key
is greater — e.g. successor(80).

**Likely questions.** Why keep a candidate? (Going left can still find a
smaller valid answer; going right can never create a new candidate.)
Difference to the parent-pointer method? (Same result; top-down needs no
parent walk and handles missing query keys.)

## 5. Range query with pruning

**Idea.** Inorder traversal that refuses to enter hopeless subtrees:
at a room with key k, only go left if k > lo (something ≥ lo can be
there), only go right if k < hi. Everything skipped is reported with a
HIGHLIGHT_SUBTREE "skipped" event, which the GUI greys out as "pruned".

**Worked example: range [75, 85].** At 50: 50 ≤ 75 so the whole left
half (30, 20, 40) is pruned. Right to 70: 70 ≤ 75, prune 60. Right to
80: collect 80. 3 rooms visited instead of 7.

**Complexity.** O(h + m) where m = number of reported keys — the visited
rooms are the two boundary paths plus the in-range nodes. Output comes
out sorted because it is still an inorder traversal.

**Likely questions.** Why is the output sorted? Why is pruning safe?
(BST ordering guarantees the skipped subtree lies entirely outside the
range.) Worst case? (lo = min, hi = max → O(n), nothing prunable.)

## 6. kthSmallest (plain BST) vs kth (AVL, size-based)

**Plain BST version:** iterative inorder with a counter; stop at the
k-th visited node. O(n) time, O(h) extra space for the stack.

**AVL version using subtree sizes:** at a room, let `ls` = size of the
left subtree. If k = ls + 1 the room itself is the answer; if k ≤ ls go
left; otherwise go right searching for position k − ls − 1.

**Worked example (k = 5, sizes known).** Root 50 has ls = 3. 5 > 4, so
go right looking for position 5 − 4 = 1 → at 70, ls = 1, 1 ≤ 1 → left →
60 has ls = 0 → position 1 = 60 itself. Answer 60.

**Complexity.** O(log n) because each step descends one level of a
balanced tree. `rank(key)` is the mirror image (count rooms ≤ key while
descending), and `countInRange(lo, hi) = rank(hi) − rank(lo−1)`.

**Likely questions.** Why store size in every node? (It turns order
statistics from O(n) scans into O(log n) descents.) What keeps sizes
correct? (The same upward fix loop as heights, plus the validator.)

## 7. AVL balance factor and the four rotations

**Balance factor** BF = height(left) − height(right), with a missing
child counting 0. A node is fine at −1, 0, +1; |BF| ≥ 2 must be fixed.

The violated node's **heavy child** decides the case:

| Case | Shape | Fix |
|---|---|---|
| LL | left-heavy node, left child leans left (BF ≥ 0) | one **right** rotation at the node |
| RR | right-heavy, right child leans right (BF ≤ 0) | one **left** rotation |
| LR | left-heavy, left child leans **right** | left-rotate the child, then right-rotate the node |
| RL | right-heavy, right child leans **left** | right-rotate the child, then left-rotate the node |

**Rotation mechanics (right rotation at x, left child y):** y's right
subtree becomes x's left subtree; x becomes y's right child; y takes x's
place under x's old parent. Inorder order is unchanged — that is the
whole point. Afterwards only x and y (and ancestors) need their height
and size recomputed, x first because it is now below y.

**Worked example (LL):** insert 30, 20, 10. After 10: BF(30) = 2, left
child 20 has BF 1 → LL → right rotation at 30 → root 20 with children
10 and 30. Height drops from 3 to 2.

**Worked example (LR):** insert 30, 10, 20. BF(30) = 2 but child 10
leans right (BF −1). Single rotation would not fix it: first left-rotate
10 (lifting 20), the shape becomes the LL picture, then right-rotate 30.
Root 20, children 10 and 30.

**Complexity.** One rotation is O(1) pointer changes + O(h) height fixes
on the way up.

**Likely questions.**
1. *Why does a single rotation fail in LR?* The deep grandchild (20) is
   in the middle; a right rotation at 30 would just move the strain to
   the other side. The first rotation straightens the path.
2. *How many rotations can an insert need?* At most one single or one
   double — fixing the lowest violated node restores every ancestor.
3. *And a delete?* Up to O(log n) rotations: shrinking a subtree can
   violate ancestors repeatedly, so we keep checking up to the root.
4. *The deletion special case?* When the heavy child has BF 0, both LL
   and "LR" conditions look possible; our rule `BF(child) ≥ 0 → single`
   handles it with one rotation (tested explicitly).
5. *Why is the AVL height O(log n)?* Each node's subtrees differ by at
   most one level, which forces at least Fibonacci-many nodes per
   height; inverting gives h ≤ 1.44·log2(n+2).

## 8. Insert/delete rebalancing walk

After every structural change we loop from the changed node's parent up
to the root; at each node we recompute BF (from stored child heights)
and repair with the table above. One loop handles inserts (finds at most
one repair) and deletes (may find several). Our `predictInsert` runs the
same logic on a silent copy of the tree, so level 3.4 can quiz the
player without touching the real vault.

## 9. The event system

Model operations append `Event` objects (type + payload + text line)
into an `EventLog`. Structural events carry a **snapshot index**: the
whole tree shape (ids, keys, child/parent ids, heights, sizes) saved in
preorder right after the change. The animation layer lays out the next
snapshot and slides every room (matched **by id**) from its old to its
new position — the GUI contains no tree logic at all, which is why the
model is fully unit-testable and the animation can replay, pause, step
and scrub. With `setEventLog(nullptr)` (benchmarks) nothing is recorded.

**Likely questions.** Why snapshots instead of animating pointer moves?
(One general mechanism covers insert, delete, swap and all rotations.)
Cost? (O(n) per snapshot, bounded by MAX_SNAPSHOTS; only GUI operations
pay it, never benchmarks.)

## 10. Undo

Before each player action the level stores a deep copy (`copyFrom`) of
the tree. `copyFrom` keeps every node's id and the tree's `nextId`, so a
restored tree is indistinguishable from the original — a later insert
creates the same id either way. Z restores the copy and the view snaps
to the restored layout (no tween), so no room is ever left at a stale
position. The spec's inverse-event undo is an allowed upgrade; snapshots
were chosen as the simpler, safer option.

**Likely questions.** Why is keeping `nextId` important? (Otherwise a
redo-style insert would mint a different id and the GUI would lose track
of the room.) Memory cost? (O(n) per saved copy; the sandbox keeps a
small fixed stack of 16 copies.)

## 11. Complexity summary table

| Operation | BST (balanced / chain) | AVL |
|---|---|---|
| search | O(log n) / O(n) | O(log n) |
| insert | O(log n) / O(n) | O(log n), ≤ 1 rotation |
| delete | O(log n) / O(n) | O(log n), ≤ O(log n) rotations |
| min/max, successor | O(log n) / O(n) | O(log n) |
| rangeQuery | O(h + m) | O(log n + m) |
| kth / rank | O(n) scan | O(log n) via sizes |
| space | O(n) | O(n) |

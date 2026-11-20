# Design decisions

Places where the spec allowed more than one reading, and what we chose.
(Spec rule 12 in CLAUDE.md: pick the simplest sensible interpretation and
write it down here.)

## Alarm counts rooms, not individual comparisons
The spec says both "entering a room or comparing costs 1 alarm step" and
"optimal = searchPathLength(target)" (the number of rooms on the path).
Since every room entry comes with exactly one key comparison, we charge
**one alarm step per room entered** (one per VISIT event). This keeps
optimal = path length and the star formula consistent. COMPARE events are
still emitted separately so the event log reads naturally.

## Height convention
Leaf height = 1, empty tree height = 0 (the official rule). The balance
factor uses the same stored heights. Sorted inserts of n keys therefore
give height exactly n (checked by a test).

## Validator checks local height/size consistency
Instead of recomputing heights with a second traversal (O(n^2) in the
10,000-operation stress test), the validator checks at every node that
`stored height == 1 + max(children's stored heights)` and the same for
size, plus leaf values. If that holds at every node, the stored values
are globally correct by induction from the leaves. O(n) and simple to
explain in the viva.

## Undo = snapshot, not inverse events
The spec allows snapshots as the simple alternative. Before each player
action a level stores `copyFrom` of the tree; Z restores it and the view
snaps to the restored layout (no animation), so nothing is left in a
wrong place. Ids survive because `copyFrom` keeps ids and `nextId`.

## Delete uses the inorder successor and swaps values
In the two-child case the keys and treasures are swapped (SWAP_VALUE
event), then the successor node is unlinked. Node ids do not move, which
is what lets the GUI animate the value swap.

## successor()/predecessor() work for any query key
They do a top-down search for the smallest key greater (largest smaller)
than the query, so the query key itself does not have to exist. This is
simpler than the parent-pointer walk and is what level 2.x needs.

## lcaBST requires both keys to exist
Checked silently first; the walk itself is the standard ordering descent.

## Scripted level keys convention
In a level JSON with `generator: "explicit"`, the first `n` keys build
the starting tree and the remaining keys are scripted later inserts
(Act 3 uses them for repair waves and prediction rounds). If `n` is 0,
all keys build the starting tree.

## AVL rebalancing walks the whole path up
After insert AND delete we walk from the changed node's parent to the
root and repair every violated node (delete can need several rotations;
for insert the loop simply finds nothing more after the single repair).
One loop for both cases keeps the code small.

## predictInsert simulates on a copy
It copies the tree (ids preserved), inserts silently with rotations off,
and reads which case the first unbalanced ancestor would need. The real
tree is untouched (checked by a test).

## Boss 2.5 capacity shrink rate
"Shrinking alarm capacity" is implemented as capacity -1 every second
player step (never below alarm+1), which the winnability test proves is
beatable for n = 10 with perfect play.

## Random generator keys
`generator: "random"` spreads n distinct keys evenly over
[keyMin, keyMax] and shuffles them with the level seed, so every run of a
level is identical and gradable.

## Tests count 10 level files
`levels.txt` currently lists the ten Act 2-3 levels; Partner B appends
Act 1 and Act 4 lines (the loader skips `#` comments, so the acts are
already marked).

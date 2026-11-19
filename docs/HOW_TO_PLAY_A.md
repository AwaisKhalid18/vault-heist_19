# How to play — Acts 2 and 3 (Partner A levels)

Every level is winnable with perfect play well inside its alarm capacity.
Optimal step counts are computed by the model, never typed in by hand.

## Controls that work everywhere

| Input | Action |
|---|---|
| Click a room | Select / answer / move into it |
| Hover a room | Tooltip: key, height, balance factor, size, depth |
| Drag child room onto its parent | Rotate (Act 3 only) |
| Wheel / drag background / **F** | Zoom / pan / fit the tree to the view |
| **Space**, **←**, **→** | Play-pause, step back, step forward through the animation |
| **Z** | Undo the last action |
| **H** | Hint (caps the level at 2 stars) |
| **Esc** | Leave the level |
| **F12** | Save a screenshot into `screenshots/` |

## Scoring

Alarm capacity = `ceil(1.5 x optimal) + 2`. Fill the meter and you fail.

| Result | Stars |
|---|---|
| Steps ≤ optimal, no penalties | 3 |
| Steps ≤ 1.25 x optimal, at most 1 penalty | 2 |
| Finished inside the capacity | 1 |
| Alarm full | 0, retry |

Score = `1000 − 10 x alarmSteps − penalties + bonuses`, floored at 0.

---

## 2.1 Search Heist — BST search

**Read:** the objective names a target key. That room also wears a gold
double ring and a gold diamond. Your current room has a pulsing blue ring
and starts at the root.

**Do:** at each step click the correct **child** of your current room.
Compare the target with the room you are standing in: target smaller, go
to the **left** child; target larger, go to the **right** child.

**Win:** step into the target room.

**Costs:** every room you enter is +1 alarm. A wrong child is +1 alarm and
moves you nowhere. Clicking a room that is not a child just shows a note.

**3 stars:** take only the correct turns and do not press H.

---

## 2.2 Stash Loot — BST insert

**Read:** the objective names the treasure to stash, e.g. "Stash treasure
48 (1 of 3)".

**Do:** walk the insert path exactly as in a search — click the correct
child each time. When the side you need is **empty**, a translucent gold
**ghost door** carrying the new key appears just below your room. Click
that ghost door and the room is built there.

**Win:** stash all three treasures.

**Costs:** +1 per room entered, +1 per room created, +1 alarm for a wrong
turn.

**Note:** you never drag anything in this level. Click the children, then
click the ghost door.

---

## 2.3 Steal and Collapse — BST delete

**Read:** a gold-ringed room is about to be stolen, and a dialog asks
which delete case applies.

**Do:** count the corridors leaving that room **downwards**:

| Corridors down | Answer |
|---|---|
| 0 | **Leaf** |
| 1 | **One child** |
| 2 | **Two children** |

Pick the matching button and the collapse animates. In the two-children
case watch the inorder successor (smallest key in the right subtree) light
up and swap its value in before the old room is unlinked.

**Win:** three rooms stolen. The level deliberately gives you one of each
case in order: leaf, one child, two children.

**Costs:** +1 alarm per room the delete walks through; a wrong case is +1
alarm and the question is asked again.

---

## 2.4 Range Raid — range query with pruning

**Read:** the objective gives a range, e.g. `[35, 65]`, and how many rooms
hold wanted loot.

**Do:** click every room whose key is inside the range. Collected rooms
turn gold. Collect them in **ascending key order** to walk the same route
the pruned query takes.

**Win:** collect them all. The pruned query then replays by itself and
greys out, with a "pruned" label, the wings a smart thief would never have
entered.

**Costs:** +1 per collected room; clicking a room outside the range is +1
alarm.

**Bonus:** no penalties and steps within optimal earns **+100**.

---

## 2.5 Boss: The Lazy Architect — degeneration

**Phase 1** plays by itself: ten deliveries arrive in sorted order and
every one hangs off the right, so the vault grows into a single corridor
of height 10. Just watch.

**Phase 2:** a deep target is marked. There is only one path, so click
straight down the chain. **The capacity itself shrinks by 1 every second
step**, so a misclick hurts twice.

**Win:** reach the target. A cutscene panel then explains why sorted
inserts cost O(n) and why Act 3 balances itself. Click through it.

---

## 3.1 Read the Badges — balance factor

**Read:** badges here show each room's **height**, not its balance factor.

**Work out** for each room: `BF = height(left child) − height(right child)`,
counting a missing child as 0. A room is healthy at −1, 0 or +1.

**Do:** click every room with `|BF| ≥ 2` (clicking toggles the selection),
then press **Submit** in the right-hand panel.

**In this tree** (50, 30, 70, 20, 80, 10, 90 inserted with balancing off)
the two strained rooms are **30** and **70**: each has a grandchild on one
side and nothing on the other.

**Win:** select exactly those two and submit. The badges then flip to
coloured balance factors.

**Costs:** +1 alarm per click; submitting with n rooms wrong or missed is
+n alarm.

---

## 3.2 Single Rotations — LL and RR

**Read:** badges show balance factors. The room to repair wears the gold
ring — always the **deepest** strained room.

**Do:** drag a child room and drop it **onto its own parent**:

| You drag | Rotation performed | Fixes |
|---|---|---|
| the **left** child up | right rotation | **LL** |
| the **right** child up | left rotation | **RR** |

While dragging, a gold line follows the cursor and a label says which
rotation the drop would perform, so you can check before releasing.

Pick the side the strain is on: a left-heavy room whose left child also
leans left is LL, so drag that left child up.

**Win:** after the vault is calm, two new deliveries arrive and strain it
again. Keep repairing until the script runs out.

**Costs:** a wrong rotation is **+2 alarm and −50 points**.

---

## 3.3 Double Rotations — LR and RL

**Read:** these strains cannot be fixed with one rotation, because the
heavy child leans the *opposite* way.

**Step 1 — name the case** in the dialog:

| Shape | Case |
|---|---|
| Room is left-heavy, its **left** child leans **right** | **LR** |
| Room is right-heavy, its **right** child leans **left** | **RL** |

**Step 2 — two drags, in order:**

- **LR:** first drag the heavy left child's **right** child up onto it
  (that straightens the path), then drag the strained room's new **left**
  child up onto the strained room.
- **RL:** first drag the heavy right child's **left** child up onto it,
  then drag the strained room's new **right** child up onto the strained
  room.

A message after you name the case reminds you which child to drag first.

**Costs:** a wrong name or a wrong rotation is **+2 alarm and −50**.

---

## 3.4 Auto Mode and Prediction

**Read:** before each delivery a dialog asks which repair the insert will
trigger: **None / LL / RR / LR / RL**.

**Work it out:**
1. Trace where the new key lands (ordinary insert path).
2. Walk back up and find the **lowest** room whose balance factor would
   reach +2 or −2.
3. If that room's heavy child leans the **same** way, it is a single
   rotation (LL for left-left, RR for right-right). If it leans the
   **other** way, it is a double (LR or RL). If no room breaks, answer
   None.

**Do:** answer, then watch the vault rotate itself back into shape.

**Win:** get through all the deliveries.

**Scoring:** each correct prediction is **+50 bonus**; a wrong one is +2
alarm and −50. Undo is disabled here because the script is fixed.

---

## 3.5 Boss: Order-Statistic Heist — subtreeSize, kth, rank

**Read:** badges show each room's **subtree size**. The objective converts
the contract for you, e.g. "steal the 4th MOST valuable — the 12th
smallest of 15". Call that number **p**. The target is *not* marked: the
sizes are your only guide.

**Do:** starting at the root, let **s** be the size badge of the left
child (0 if there is none):

| Condition | Move |
|---|---|
| `p == s + 1` | you are standing on it — **click your current room** to steal it |
| `p ≤ s` | click the **left** child |
| `p > s + 1` | click the **right** child, and continue looking for position `p − s − 1` |

**Then:** a new delivery arrives, the vault rebalances, and you must
**confirm the new size label** of each rotated room from three choices —
count the rooms in that subtree, including the room itself.

**Win:** three successful rounds.

**Costs:** +1 per room entered, +1 for stealing the wrong room, +1 for a
wrong size label. Undo is disabled because the rounds are scripted.

# Level guide — how to read, change and add a level

Levels are **data, not code**. Adding one is two steps: write a `.json` file and
add a line to `assets/levels/levels.txt`. No recompile is needed as long as the
level's `type` is one a factory already knows, and §5 covers the case where it
is not.

---

## 1. Where the files live

```
assets/levels/levels.txt        the campaign, in order
assets/levels/act1/             level_1_1.json .. level_1_5.json
assets/levels/act2/             level_2_1.json .. level_2_5.json
assets/levels/act3/             level_3_1.json .. level_3_5.json
assets/levels/act4/             level_4_1.json .. level_4_5.json
```

`levels.txt` is one file path per line, in the order the campaign plays them.
`#` starts a comment.

```
# Act 1 (Partner B)
assets/levels/act1/level_1_1.json
assets/levels/act1/level_1_2.json
...
```

The campaign map reads this file, groups the levels it finds by their `act`
field, and unlocks each one when the level before it has been cracked.

---

## 2. What a level file looks like

One **flat** JSON object. It is read by our own `LevelLoader`, not a JSON
library, so it understands exactly three kinds of value: a string, an integer,
and an array of integers or of strings. No nesting, no floats, no `true`/`false`,
no `null`.

```json
{ "id": "4.1", "act": 4, "title": "Activate the Vault",
  "mission": "Organise the treasure into a Max Heap",
  "concept": "Complete binary tree; Max Heap property; bottom-up heap construction",
  "briefing": "The final vault answers to priority, not to order...",
  "type": "heap_build", "seed": 4101,
  "generator": "random", "n": 10, "keyMin": 10, "keyMax": 99, "keys": [],
  "k": 0, "lo": 0, "hi": 0,
  "hints": ["Work upward from the last parent, index n/2 - 1.",
            "A room marked with ! is beaten by one of its children."] }
```

### Every field

| Field | Type | What it does |
|---|---|---|
| `id` | string | `"4.1"`. Shown everywhere, and it is the key the save file records progress under, so **it must be unique**. |
| `act` | int | 1–4. Decides which column of the campaign map the level appears in. |
| `title` | string | The name on the button and in the briefing. |
| `mission` | string | One line, shown under the briefing. |
| `concept` | string | The data-structure idea. Documentation; nothing reads it. |
| `briefing` | string | The dialog shown before play. This is where the teaching happens — write it in the game's voice, not as a manual. |
| `type` | string | **Which screen to build.** See §3. An unknown type means the level cannot open. |
| `seed` | int | Seeds the level's `Random`. The same seed gives the same vault every time, which is what makes a level fair and gradable. **Give every level its own.** |
| `generator` | string | How the starting data is made. See §4. |
| `n` | int | How many rooms or treasures. |
| `keyMin`, `keyMax` | int | The value range the generator draws from. |
| `keys` | int array | Used when `generator` is `"explicit"`. Convention: the first `n` entries build the starting structure and anything after them is a scripted later insert. |
| `k` | int | A per-level number. Act 1.1 uses it for the traversal; 1.2 for which pair of traversals; 4.3 for how many treasures to take; 3.5 for the k-th. |
| `lo`, `hi` | int | The range for Act 2.4's range query. Ignored elsewhere. |
| `hints` | string array | Up to 5. **H** cycles through them; using one caps the level at 2 stars. |

Fields a level does not need are still written, set to `0` or `[]`, so every
file has the same shape and is easy to copy.

---

## 3. Level types

The `type` string is what `LevelFactory` switches on. It tries Partner A's
factory first, then Partner B's, in a plain if-chain — no maps, no function
pointers.

### Act 1 — general binary trees (`src/game/LevelFactoryB.cpp`)

| `type` | Screen | What the player does | Uses `k` |
|---|---|---|---|
| `bt_traversal` | `Level11Traversal` | Clicks rooms in the named traversal order | **yes:** 0 preorder, 1 inorder, 2 postorder, 3 level order |
| `bt_build` | `Level12Rebuild` | Places values into the right rooms from two traversal lists | **yes:** 0 preorder + inorder, 1 postorder + inorder |
| `bt_height` | `Level13Heights` | Types each room's height bottom-up, then the diameter | no |
| `bt_lca` | `Level14Lca` | Finds where two routes meet, then traces the path to it | no |
| `bt_mirror` | `Level15Mirror` | Clicks rooms to swap their children until the vault is mirrored | no |

### Act 4 — Max Heap and Heap Sort

| `type` | Screen | What the player does | Uses `k` |
|---|---|---|---|
| `heap_build` | `Level41Build` | Swaps a parent with one of its children until the heap property holds | no |
| `heap_repair` | `Level41Build` | The same move, on a heap that starts broken | no |
| `heap_extract` | `Level43Extract` | Takes the root `k` times | **yes:** how many treasures |
| `heap_race` | `Level43Extract` | Takes the root until the heap is empty | no |
| `heap_boss` | `Level45Boss` | Builds a Max Heap, then sorts it completely | no |

Two pairs share a screen (`heap_build`/`heap_repair`, `heap_extract`/`heap_race`)
because the player's move is identical in each pair — only the starting data and
the alarm budget differ, and both of those come from the file.

### Acts 2 and 3 — BST and AVL (`src/game/LevelFactoryA.cpp`)

`bst_search`, `bst_insert`, `bst_delete`, `bst_range`, `bst_boss_degenerate`,
`avl_badges`, `avl_single_rot`, `avl_double_rot`, `avl_predict`, `avl_boss_kth`.

---

## 4. Generators

`generator` says how the starting data is built from `seed`, `n`, `keyMin` and
`keyMax`.

### Act 1 trees

| `generator` | Produces |
|---|---|
| `random` | `n` distinct values in `[keyMin, keyMax]`, arranged in a **random shape** |
| `explicit` | the values from `keys`, again in a random shape |

Act 1 trees are **not** search trees — there is no ordering rule, which is the
whole point of the act. A random shape is made by treating the value list as an
inorder walk and picking a random root inside each range to build the preorder;
`LevelBuilderB::buildTree` does this with an explicit stack of ranges. The values
are kept distinct, because a pair of traversals only describes one tree when
they are.

The spec asks for Act 1 trees of 6 to 15 rooms, and a unit test enforces it.

### Act 4 arrays

| `generator` | Produces | Good for |
|---|---|---|
| `random` | `n` random values in `[keyMin, keyMax]` | 4.1, 4.5 — an unordered vault |
| `heap` | random values, then built into a valid Max Heap | 4.3, 4.4 — extraction starts from a working mechanism |
| `disturbed` | a valid Max Heap, then a few parent/child pairs swapped back out of place | 4.2 — repair |
| `sorted` | ascending | the worst case for repeated insertion |
| `reverse` | descending, which is **already** a valid Max Heap | showing why input order matters |
| `duplicates` | a small range, so ties are everywhere | proving ties are handled |
| `explicit` | exactly the values in `keys` | a hand-made case you want to demonstrate |

`disturbed` only ever swaps a parent with **its own child**, so the damage can
always be undone by legal player swaps. That is what keeps level 4.2 solvable by
hand rather than only by a full rebuild.

---

## 5. Adding a level

**Using a type that already exists** — no code at all:

1. Copy the closest existing file in `assets/levels/actN/`.
2. Change `id` (must be unique), `title`, `mission`, `briefing`, `hints`.
3. Give it **a new `seed`**, or it will be the same vault as the file you copied.
4. Choose `generator` and `n`.
5. Add its path to `assets/levels/levels.txt` where it should sit in the order.
6. Run `./build.ps1 test` — the level tests load every file in the list and prove
   the new one is winnable inside its own alarm capacity.

**Adding a new type** — three small steps:

1. Write the screen, deriving from `TreeLevelBase` (Act 1 style) or
   `HeapLevelBase` (Act 4 style). The only method you must write is
   `onLevelStart()`: build the structure, call `setObjective(...)` and set
   `optimalSteps`. Override `onNodeClicked` / `onCellClicked` and `onOperation`
   for the interaction.
2. Add one `if (def.type == "your_type") { return new YourScreen(app, def); }` to
   `src/game/LevelFactoryB.cpp`.
3. Write the level file and list it.

New `.cpp` files under `src/screens/` are picked up automatically — CMake globs
with `CONFIGURE_DEPENDS`, so you do not edit `CMakeLists.txt`.

---

## 6. The rule about optimal steps

**`optimalSteps` is never written into a level file.** Every level computes it by
asking the model what a perfect play costs:

| Level | How the optimum is computed |
|---|---|
| 1.1 | the length of the traversal `BinaryTree` returns |
| 1.2 | one placement per room |
| 1.3 | one answer per room, plus one for the diameter |
| 1.4 | one guess, plus the length of `pathTo(lca)` |
| 1.5 | the number of rooms that actually have a child to swap |
| 4.1, 4.2 | the swaps `MaxHeap::buildMaxHeap` performs on that exact array |
| 4.3 | the number of extractions asked for |
| 4.4 | one extraction per treasure |
| 4.5 | the build swaps plus one extraction per treasure |

The alarm capacity is then `ceil(1.5 × optimalSteps) + 2`. This is why a level
can never be accidentally impossible, and why changing `n` or `seed` is safe:
the budget moves with the data. `tests/test_levels_b.cpp` plays every Act 1 and
Act 4 level perfectly and asserts the result fits inside the capacity, so a bad
level fails the build rather than reaching a player.

---

## 7. Writing a good briefing

The briefing is the only place a level teaches, and it is read once. What works:

- **Say the rule in the story's voice.** "Every chamber must hold at least as
  much as the two below it" lands better than "maintain the max-heap property".
- **Name the mechanism.** Say that index `i` has its children at `2i+1` and
  `2i+2`; the player is looking at both views and the connection should be
  stated, not left to be guessed.
- **Say what a step costs**, if the level charges for something unusual.
- **Keep hints laddered.** The first nudges, the second names the technique, the
  third nearly gives it away — they are revealed one at a time.

---

## 8. Troubleshooting

| What you see | What it is |
|---|---|
| The level is missing from the campaign map | its path is not in `levels.txt`, or the line is commented out |
| Clicking the level does nothing | `type` matches no factory entry — check the spelling against §3 |
| The level opens empty | `n` is 0, or `generator` is `explicit` with an empty `keys` |
| The alarm is full before you start | `optimalSteps` came out 0; the screen clamps it to 1, so check what `onLevelStart` computed |
| Two levels share progress | they have the same `id` — the save file keys on it |
| The vault is identical to another level | they share a `seed` |

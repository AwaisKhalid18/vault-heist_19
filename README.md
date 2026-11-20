# Vault Heist: The Living Vault

A C++17 + SFML puzzle-strategy game for a Data Structures course. The map is a
tree of vault rooms, and every step the thief takes raises an alarm meter — so
the **shape** of the tree is what decides whether you escape. Four acts, twenty
levels, one data structure each.

| Act | The wing | What it teaches |
|---|---|---|
| 1 | The Blueprint Wing | General binary trees: the four traversals, reconstruction, height, diameter, LCA, mirroring |
| 2 | The Sorted Wing | Binary Search Trees: search, insert, the three delete cases, range queries, degeneration |
| 3 | The Architect's Wing | AVL trees: balance factors, all four rotations, rebalancing inserts, order statistics |
| 4 | The Heap Fortress | Max Heaps and Heap Sort: bottom-up construction, heapify, extract-max, the full sort |

Every data structure is implemented from scratch. There is no STL container
anywhere in the project, nothing from `<algorithm>`, and no library tree, heap
or priority queue — see [Constraints](#constraints).

---

## Running it

**You need:** Windows, a C++17 compiler (MSVC from Visual Studio Build Tools),
and CMake. **SFML is not something you install** — CMake downloads and builds
SFML 2.6.2 automatically the first time you configure, so the first build takes
a few minutes and later ones take seconds.

```powershell
./build.ps1          # configure + build (Release, Ninja, MSVC)
./build.ps1 run      # build, then play the game
./build.ps1 test     # build, then run the unit tests
./build.ps1 bench    # build, then run the benchmarks
./build.ps1 clean    # delete the build folder
```

`./build.ps1 run` opens the main menu. To jump straight to one level for a demo,
pass its file:

```powershell
./build/vault_heist.exe assets/levels/act4/level_4_1.json
```

**If the build fails with `LNK1104 cannot open file 'vault_heist.exe'`**, the
game is still running. Close it, or:

```powershell
Get-Process vault_heist,dev_a -ErrorAction SilentlyContinue | Stop-Process -Force
```

### What gets built

| Target | What it is |
|---|---|
| `vault_heist` | the game |
| `run_tests` | the unit test suite — 366 checks, no framework |
| `run_bench` | the benchmarks, writing CSV into `bench/results/` |
| `dev_a` | a developer launcher that lists every level directly |

---

## Controls

**Everywhere**

| Key | Does |
|---|---|
| Mouse | click to select or answer |
| Esc | back one screen |
| F12 | save a screenshot into `screenshots/` |

**In a level**

| Input | Does |
|---|---|
| Click a room / array cell | select it, or answer the level's question |
| Hover a room | tooltip: value, height, size, depth — or index, parent and children in Act 4 |
| Mouse wheel | zoom at the cursor |
| Drag the background | pan the canvas |
| **F** | fit the whole structure on screen |
| **Space** | play / pause the animation |
| **← / →** | step one event back or forward |
| **Z** | undo the last move |
| **H** | show the next hint (costs a star) |
| Drag a child onto its parent | perform that rotation (Act 3) |

The toolbar along the bottom has the same controls as buttons, plus a speed
slider (0.25× to 4×) and a scrubber for replaying the last operation.

---

## How to play

The **alarm meter** is the whole game. Entering a room or comparing against a
room's key costs one alarm step, and each level's capacity is
`ceil(1.5 × optimalSteps) + 2`. Fill the meter and you are caught.

**The optimal step count is computed by the model, never typed into a level
file.** Level 4.1 asks `MaxHeap` how many swaps bottom-up construction needs on
its own array; level 1.1 asks `BinaryTree` how long the traversal is. That is
why the alarm budget is always fair, and why a level can never be impossible.

**Stars**

| Result | Stars |
|---|---|
| Steps ≤ optimal, no penalties | 3 |
| Steps ≤ 1.25 × optimal, at most 1 penalty | 2 |
| Finished within the alarm capacity | 1 |
| Alarm full | 0 — retry |

Using a hint caps you at 2. `Score = 1000 − 10 × alarmSteps − penalties + bonuses`,
floored at 0. Penalties: a wrong traversal click costs +1 alarm, a wrong
rotation +2 alarm and −50 points, an invalid Act 4 swap +1 alarm.

**Modes.** *Campaign* walks the four acts in order, unlocking each level as you
crack the one before it. *Endless* is a survival run on the Heap Fortress:
treasures keep arriving and you extract maxima to stop the vault overflowing,
scored on how long you last. *Sandbox* lets you run every operation on any tree
or heap, outside the campaign — which is the quickest way to see an animation
you want to look at again. *Settings* can unlock every level for a demo.

Progress, the leaderboard and the settings are saved to `vault_heist_save.txt`
next to the executable.

---

## How it is put together

The guiding rule is that **the data structures know nothing about the GUI.**

```
Model (pure C++)  ->  EventLog  ->  Animation  ->  Renderer
     ^                                                 |
     |                                                 v
  Level logic   <-------------------------------   your clicks
```

A model operation never draws. It writes `Event` objects into an `EventLog`
handed to it with `setEventLog(log)`, and the GUI replays that log one event at
a time. If the pointer is `nullptr` — which is what the benchmarks do — no
events are written at all. Node movement is not animated by GUI logic either:
the log stores **snapshots** of the whole structure, and the animator slides
each node by its id from its place in one snapshot to its place in the next. A
rotation is just "everyone moves to the next snapshot", so there is no rotation
code in the renderer.

| Folder | What lives there |
|---|---|
| `include/core`, `src/core` | `Event`, `EventLog`, `NodeStack`, `NodeQueue`, `Random`, `TextUtil`, `Limits` |
| `include/model`, `src/model` | `BinaryTree`, `BST`, `AVLTree`, `MaxHeap` and their validators — **no SFML, fully unit-testable** |
| `include/game`, `src/game` | level definitions and loading, scoring, the campaign list, save files |
| `include/gui`, `src/gui` | camera, layout, renderers, animation, and eleven reusable widgets |
| `include/screens`, `src/screens` | every menu and every level |
| `tests/` | the unit test suite |
| `bench/` | the benchmarks |
| `docs/` | the report, test report, benchmark analysis, viva notes, level guide |

Two things are worth knowing if you read the code:

- **Node ids are never reused.** The GUI tracks rooms by id across rotations,
  copies and value swaps, so an id that came back would make a node teleport.
- **`NodeStack` and `NodeQueue` exist because `std::stack` and `std::queue` do
  not.** They are what the iterative traversals and the snapshot writer run on.

---

## Constraints

These come from the course handout and are not style preferences.

**Not used anywhere:** any STL container (`vector`, `map`, `set`, `stack`,
`queue`, `deque`, `priority_queue`, `array`, `pair`, `tuple`, …), anything from
`<algorithm>`, `enum` or `enum class`, global variables, templates, lambdas,
`auto`, smart pointers, `std::function`, `std::optional`, move semantics,
operator overloading, exceptions, `<chrono>`, `<random>`, `<filesystem>`, any
JSON library, or any test framework.

**Used instead:** `std::string` for text, SFML types for the GUI and for timing
(`sf::Clock`, since `<chrono>` is out), `<cmath>`, `<iostream>`, `<fstream>`
and `<sstream>`, named `static const int` members inside classes in place of
enums, a hand-written JSON reader for the level files, our own seeded linear
congruential `Random` instead of `rand()`, and our own `TestRunner`.

"Not found" is reported as a `bool` with an `int&` out-parameter. A list of
results is written into a caller-provided array and the count returned. Leaf
height is 1 and an empty tree has height 0, so inserting sorted keys into a
plain BST really does give height n — which is the point level 2.5 makes.

The whole project can be checked with three greps:

```bash
grep -rnE "\b(vector|map|set|stack|queue|deque|priority_queue|sort|make_heap)\s*<" src include
grep -rn "\benum\b" src include
grep -rn "\bauto\b" src include
```

---

## Testing and benchmarks

```powershell
./build.ps1 test
```

**366 checks, 0 failures**, with no test framework — `tests/TestRunner.h` is a
class with a `check(bool, name)` method that prints `[PASS]` or `[FAIL]` and
counts. Output is mirrored into `docs/test_output_A.txt`.

The suite covers every case the spec asks for: empty, single-node, skewed and
full trees; recursive traversals checked against iterative ones; reconstruction
round trips; all three BST delete cases; one test per AVL rotation; order
statistics against a sorted array; heap construction from empty, single,
duplicate, already-heap and reverse-sorted input; Heap Sort against an
independent reference sort; and save-file round trips. Two long random runs back
the rest up: **10,000 seeded heap operations with the validator run after every
single step**, and 300 random tree shapes checked for traversal agreement.

```powershell
./build.ps1 bench
```

Benchmarks run without the GUI and without event recording. They write CSV into
`bench/results/` for the report's charts. The headline numbers:

- **Bottom-up heap construction is O(n)** — 190, 1,982 and 19,982 comparisons at
  n = 100, 1,000 and 10,000, against 480, 7,987 and 113,631 for inserting the
  same values one at a time.
- **Heap Sort is O(n log n)** — comparisons divided by `n·log₂n` stay between
  1.57 and 1.95 at every size and on every input shape.
- **A sorted BST degenerates and an AVL does not** — height 1,000 against height
  10 for 1,000 sorted keys.

Full tables and the plain-English explanation are in
`docs/BENCH_RESULTS_A.md` and `docs/BENCH_RESULTS_B.md`.

---

## Adding a level

Levels are data, not code: a level file plus a line in `assets/levels/levels.txt`
is enough, as long as its `type` is one a factory already knows.
`docs/LEVEL_GUIDE.md` explains every field and lists every level type.

---

## Who wrote what

A two-person project, split down a shared contract — the headers in `include/`.

**Partner A:** the shared core (events, event log, containers, scoring, level
loading, `App`, `Theme`), `BST`, `AVLTree`, `TreeValidator`, the whole rendering
and animation engine, the eleven widgets, Acts 2 and 3, the BST/AVL sandbox, and
the tree benchmarks.

**Partner B:** `BinaryTree`, `MaxHeap`, `HeapValidator`, the heap animator and
the synchronised tree and array views, Acts 1 and 4, the main menu, campaign
map, result screen, settings, leaderboard, endless mode, the Binary/Heap
sandbox, `SaveManager`, and the heap and sorting benchmarks.

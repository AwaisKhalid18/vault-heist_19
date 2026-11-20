# Partner B handoff — Vault Heist: The Living Vault

**Read this whole file before writing any code.** You are picking up an
existing, working C++17 + SFML project. Partner A's half is finished,
tested and pushed. Your job is **Partner B's half only**.

> **Deadline: 20 Nov 2026, 11:59 PM.** This is tight. Build in the phase
> order in §9 and commit after every phase, so that even a partial Act 4 is
> submittable. Working, simple, tested code beats clever code.

---

## 1. What the project is

A C++/SFML game for a second-year Data Structures course. The game map is a
tree of vault rooms; every move the player makes costs "alarm". Each act
teaches one structure:

| Act | Structure | Owner |
|---|---|---|
| 1 | General binary trees (traversals, build-from-traversals, LCA, mirror) | **B (you)** |
| 2 | Binary Search Trees | A — done |
| 3 | AVL trees | A — done |
| 4 | Max Heap + Heap Sort | **B (you)** |

It is a two-person project. The two halves meet at a **shared contract**:
the headers in `include/`. Partner A has already written every shared
header and left you compiling stubs for your files, so the project builds
and links right now, today, before you write a line.

**Repo:** `https://github.com/AwaisKhalid18/vault-heist.git`
**Branches:** `main`, `awais` (Partner A), `ali` (Partner B — yours).
Partner A's work is merged into `main`.

---

## 2. THE HARD CONSTRAINTS — read twice, breaking these costs marks

These come from the official course handout. They are not style
preferences. The official course handout is the authoritative copy, and
the Constraints section of README.md restates them; this is a summary.

| Rule | Detail |
|---|---|
| **No STL containers** | No `std::vector`, `list`, `map`, `set`, `unordered_map`, `stack`, `queue`, `deque`, `priority_queue`, `array`, `pair`, `tuple`. |
| **Nothing from `<algorithm>`** | No `sort`, `swap`, `max`, `min`, `make_heap`. Write your own — a two-line `swapInts(int& a, int& b)` helper inside your class is the expected answer. |
| `std::string` is allowed | For text. |
| SFML types are allowed | `sf::Vector2f`, `sf::Text`, `sf::Color`, `sf::Event`, `sf::Clock`, `sf::VertexArray` etc. You must be able to explain each one in a viva. |
| C standard headers allowed | `<cmath>`, `<cstdlib>`, `<iostream>`, `<fstream>`, `<sstream>`. |
| `using namespace std;` allowed | — |
| **No global variables** | Every variable lives inside a function or a class. No global counters, constants or objects. |
| **NO ENUMS AT ALL** | Neither `enum` nor `enum class`, anywhere. Use `static const int` members **inside a class**: `static const int PHASE_BUILD = 0;` and store the value in an `int`. (SFML's own built-in values like `sf::Keyboard::Z` are fine — you just never *declare* an enum.) |
| **Leaf height = 1** | Empty tree height = 0. This matters: sorted inserts of n keys give height n. |

**Also forbidden, because we must explain every line in a viva:**
templates, lambdas, `auto`, `std::optional`, `std::function`, smart
pointers, move semantics, operator overloading, exceptions,
`std::filesystem`, `<chrono>`, `<random>`. No library other than SFML — no
JSON library, no test framework, no Python.

**Instead of the banned things, the house style is:**
- "Not found" → return `bool`, write the answer through an `int&`
  out-parameter: `bool extractMax(int& result);`
- "A list of results" → caller passes an array and a maximum, you return
  the count: `int preorderIter(int outIds[], int maxCount);`
- Random numbers → the project's own `Random` class (`include/core/Random.h`),
  a seeded linear congruential generator. **Never** `rand()` or the clock.
- Timing → `sf::Clock` (because `<chrono>` is banned).

**Formatting (match the existing code exactly):**
- 4-space indent, **Allman braces** (opening brace on its own line).
- One statement per line; space after `if`/`for`/`while`.
- Blank line between functions. Private members at the **bottom** of the class.
- One class per `.h`/`.cpp` pair. Clear English names (`siftDown`, `heapSize`).
- Functions under about 40 lines; split out helpers if longer.
- **Comments only where they add value** — above a non-obvious function, or
  inline on tricky lines (heap index maths, pointer rewiring, layout maths).
  Never comment the obvious, never write banner comments, never leave
  commented-out code.
- Always `delete` what you `new` (destructors and `clear()`).

**Self-check before you commit.** These greps must come back empty of your
own code:

    grep -rnE "\b(vector|map|set|stack|queue|deque|priority_queue|sort|make_heap)\s*<" src include
    grep -rn "\benum\b" src include
    grep -rn "\bauto\b" src include

---

## 3. Build, run, test

Windows + PowerShell. SFML 2.6.2 is fetched and built automatically by
CMake the first time you configure, so nothing needs installing but a C++
compiler and CMake.

    ./build.ps1          # configure + build (Release, Ninja, MSVC)
    ./build.ps1 run      # build then launch the full game (apps/main.cpp — yours)
    ./build.ps1 dev      # build then launch Partner A's dev launcher
    ./build.ps1 test     # build then run the unit tests
    ./build.ps1 bench    # build then run the benchmarks
    ./build.ps1 clean    # delete build/

**Gotcha that will bite you:** if the game is still running, the linker
fails with `LNK1104 cannot open file 'vault_heist.exe'`. Kill it first:

    Get-Process vault_heist,dev_a -ErrorAction SilentlyContinue | Stop-Process -Force

`CMakeLists.txt` globs sources with `CONFIGURE_DEPENDS`, so **new `.cpp`
files are picked up automatically** — you do not edit CMake to add a file.
Targets: `vault_heist` (final game), `dev_a` (A's launcher), `run_tests`,
`run_bench`.

**Build and run the tests after every change.** Never commit code that
does not compile or that fails tests. Partner A's suite is currently
**173 passed, 0 failed** — keep it that way.

---

## 4. The repo as it stands

    CMakeLists.txt  build.ps1  README.md  .gitignore
    apps/main.cpp        <-- YOURS (currently a stub that opens A's dev menu)
    apps/dev_a.cpp       <-- A's launcher, do not touch
    assets/fonts/        Cinzel.ttf, Inter.ttf, JetBrainsMono.ttf (+ OFL licences)
    assets/levels/levels.txt
    assets/levels/act1/  <-- EMPTY, yours to fill
    assets/levels/act2/  level_2_1.json .. level_2_5.json   (A, done)
    assets/levels/act3/  level_3_1.json .. level_3_5.json   (A, done)
    assets/levels/act4/  <-- EMPTY, yours to fill
    include/core/   Limits Event EventLog NodeStack NodeQueue Random TextUtil
    include/model/  TreeNode BST AVLTree TreeValidator | BinaryTree MaxHeap HeapValidator
    include/game/   LevelDef LevelLoader LevelTreeBuilder LevelFactory ScoreSystem
    include/gui/    App Screen Theme Background Camera LayoutEngine TreeRenderer
                    AnimationController ParticleBurst  + widgets/ (11 widgets)
    include/screens/ LevelScreenBase, Level21..Level35, DevMenu, Sandbox, Stress, RotationDemo
    src/...         mirrors include/
    tests/          TestRunner.h main.cpp test_core test_bst test_avl
                    test_tree_validator test_tree_events test_levels
    bench/main.cpp  A's four benchmarks
    docs/           DESIGN_DECISIONS, HOW_TO_PLAY_A, REPORT_A, TEST_REPORT_A,
                    VIVA_NOTES_A, BENCH_RESULTS_A, README_SPEC

**Rule: only edit files you own.** The shared headers in `include/core`,
`include/model`, `include/game`, and `include/gui/Theme.h` / `Screen.h` /
`App.h` are the contract. If you genuinely need one changed, **stop and
tell the user** — Partner A has to agree, or the merge breaks.

### Missing from the repo (tell the user, do not invent them)
- `docs/Project_Deliverable_1.pdf` — the official spec. `docs/README_SPEC.md`
  is a placeholder. **If the user can give you the PDF, read it before
  anything else; where it disagrees with this file, the spec
  wins and you should say so out loud.**
- `PROMPTS_PARTNER_B.md` — if the user has a phase-plan document for
  Partner B the way Partner A had one, ask for it; it overrides §9 below.

---

## 5. What you own

| Area | Your files |
|---|---|
| Model | `BinaryTree` (Act 1), `MaxHeap` + heap sort, `HeapValidator` |
| GUI | `ArrayView` (Act 4 array cells, kept in sync with the tree picture) |
| Screens | Main menu, Campaign map, Result screen, Settings, Leaderboard, Act 1 levels, Act 4 levels, Endless mode, Sandbox Binary/Heap tabs |
| Entry point | `apps/main.cpp` (final game) |
| Levels | `src/game/LevelFactoryB.cpp` + Act 1 and Act 4 JSON files, `SaveManager` |
| Tests | `tests/test_binarytree.cpp`, `tests/test_heap.cpp`, `tests/test_save.cpp` |
| Benchmarks | Heap construction, Heap Sort, sorting comparison |
| Docs | `README.md`, `docs/LEVEL_GUIDE.md`, heap sections of the report |

You also **upgrade** `App::levelFinished` to open your Result screen
(it currently prints the outcome and pops the level — the one place in
`App` you are expected to touch; the stub is marked `// TODO(B)`).

---

## 6. The contract — exact APIs you must implement

These headers already exist with stub bodies in `src/model/`. **Implement
the bodies; do not change the signatures** — Partner A's code and the
factory dispatch depend on them.

### `include/model/TreeNode.h` (shared, read-only to you)

    struct TreeNode
    {
        int id;          // unique, never reused; the GUI tracks nodes by id
        int key;
        int treasure;
        int height;      // leaf = 1, empty = 0
        int size;        // nodes in this subtree, including itself
        TreeNode* left;
        TreeNode* right;
        TreeNode* parent;
    };

### `include/model/BinaryTree.h` — Act 1, stub at `src/model/BinaryTree.cpp`

    class BinaryTree
    {
    public:
        BinaryTree();
        ~BinaryTree();
        void setEventLog(EventLog* eventLog);
        TreeNode* getRoot();

        // traversals: fill outIds, return how many ids were written
        int preorderRec(int outIds[], int maxCount);
        int preorderIter(int outIds[], int maxCount);
        int inorderRec(int outIds[], int maxCount);
        int inorderIter(int outIds[], int maxCount);
        int postorderRec(int outIds[], int maxCount);
        int postorderIter(int outIds[], int maxCount);
        int levelOrder(int outIds[], int maxCount);

        void buildPreIn(const int pre[], const int in[], int n);
        void buildPostIn(const int post[], const int in[], int n);

        int  height();
        int  diameter();
        bool lca(int idA, int idB, int& result);
        int  pathTo(int id, int out[], int maxCount);
        void mirror();
        bool isSameTree(BinaryTree& other);
        bool isSymmetric();

    private:
        TreeNode* root;
        int nextId;
        EventLog* log;
    };

Notes:
- Both a recursive **and** an iterative version of each of the three DFS
  traversals is required by the spec — that is the whole point of Act 1.
  The iterative ones use the project's own `NodeStack`; `levelOrder` uses
  `NodeQueue` (§7).
- `nextId` is a **member** counter starting at 1. No global counter.
- `lca` returns `false` if either id is absent; otherwise writes the
  answer id into `result`.
- `pathTo` writes the root-to-node id chain into `out` and returns its
  length, or 0 if the id is not in the tree.

### `include/model/MaxHeap.h` — Act 4, stub at `src/model/MaxHeap.cpp`

    class MaxHeap
    {
    public:
        MaxHeap();
        ~MaxHeap();
        int  parentOf(int i);                 // (i - 1) / 2
        int  leftOf(int i);                   // 2i + 1
        int  rightOf(int i);                  // 2i + 2
        void setEventLog(EventLog* eventLog);
        void buildMaxHeap(const int a[], int n);
        void siftDown(int i, int heapSize);
        void heapify();
        void siftUp(int i);
        bool extractMax(int& result);
        void insert(int value);
        void heapSort(const int a[], int n, int sortedOut[]);
        bool playerSwap(int i, int j);        // the player's move in Act 4
        bool isMaxHeap();
        int  get(int i);
        int  totalCount();
        int  getHeapSize();
        void load(const int a[], int n, int heapSize);   // also used for undo
        long long getComparisons();
        long long getSwaps();
        void resetCounters();

    private:
        int* data;
        int capacity;
        int count;
        int heapSize;
        EventLog* log;
        long long comparisons;
        long long swaps;
    };

Notes:
- `data` is a plain `int*` you `new[]` and grow by doubling. Free it in the
  destructor.
- `count` is how many values are stored; `heapSize` is how much of the
  array is still "the heap" — heap sort shrinks `heapSize` while `count`
  stays put, which is exactly what the Act 4 animation shows.
- `comparisons` and `swaps` are the benchmark counters. Increment them
  inside `siftDown` and `siftUp`.
- `heapSort` must not disturb the live heap the GUI is showing: build from
  `a[]` into `sortedOut[]`.

### `include/model/HeapValidator.h`

    class HeapValidator
    {
    public:
        static bool checkHeap(MaxHeap& h, string& error);
    };

Static only, no state. Walk `i = 0` to `heapSize - 1` and confirm every
child is less than or equal to its parent; on failure write a
human-readable reason into `error` (for example
`"index 4 (90) > parent index 1 (70)"`) and return `false`. Mirror Partner
A's `TreeValidator` style — read `src/model/TreeValidator.cpp` first.

### `src/game/LevelFactoryB.cpp` — currently returns `nullptr`

    Screen* LevelFactoryB::create(App& app, const LevelDef& def)
    {
        if (def.type == "bt_traversal")
        {
            return new Level11Traversal(app, def);
        }
        ...
        return nullptr;
    }

A plain if-chain on `def.type` — no maps, no function pointers.
`LevelFactory::create` already tries A first, then you. Copy the shape of
`src/game/LevelFactoryA.cpp`.

---

## 7. Shared machinery you should reuse, not rewrite

### `NodeStack` and `NodeQueue` (`include/core/`)
Arrays of `TreeNode*` that grow with `new`. These exist precisely so your
iterative traversals are legal. Read the headers; they are tiny.

### `Random` (`include/core/Random.h`)
A seeded LCG. Construct it as a local or a member, seeded from the level's
`seed` field. Same seed gives the same level every time, which is what
makes levels fair and testable.

### `EventLog` and `Event` — **the most important thing to understand**

The model never draws and the GUI never guesses. Every model operation
writes `Event` objects into an `EventLog*` handed to it by
`setEventLog(log)`. If that pointer is `nullptr` (benchmarks do this),
**no events are written** — so guard every write with
`if (log != nullptr)`.

The GUI then replays that log one event at a time. Node movement is *not*
animated by GUI logic: the log also stores **snapshots** of the whole
structure, and the animator slides each node by `id` from its position in
one snapshot to its position in the next. That is why ids must never be
reused.

Event types are already defined in `include/core/Event.h` as
`static const int` (not an enum). The heap ones are **yours**:

    HEAP_BUILD = 10, HEAP_SWAP = 11, HEAPIFY_START = 12, HEAPIFY_DONE = 13,
    EXTRACT_MAX = 14, MARK_SORTED = 15, HEAP_SIZE_CHANGE = 16,
    HEAP_SORT_COMPLETE = 17, DONE = 18

and the tree ones you need for Act 1 are `VISIT = 0`, `COMPARE = 1`,
`HIGHLIGHT_SUBTREE = 9`, with the modes `HL_VISITED`, `HL_SKIPPED`,
`HL_RESULT`.

The relevant `Event` fields for heaps are `indexA`, `indexB`, `heapSize`,
`oldSize`, `newSize`, `snapshotIndex`, and `text` (the human-readable
Event Log line, for example `"Swap index 1 and 4"`).

**Rules you must follow:**
1. Save an array snapshot with `log->saveArray(data, count)` after every
   swap and every heap-size change, and put the returned index into that
   event's `snapshotIndex`.
2. **Every batch of events ends with a `DONE` event.** The animator waits
   for it.
3. Convention: the caller saves snapshot 0 (the state *before* the
   operation), runs the model operation, then calls
   `anim.startLog(&log, autoPlay)`.

Read `src/model/AVLTree.cpp` and `src/model/BST.cpp` to see exactly how
Partner A emits events, and copy that pattern for the heap.

### `ScoreSystem` (`include/game/ScoreSystem.h`)

    struct LevelOutcome
    {
        int alarmSteps, optimalSteps, capacity, penaltyCount, penaltyPoints, bonuses;
        bool hintUsed, completed;
    };

    alarmCapacity(optimal) = ceil(1.5 * optimal) + 2
    computeStars(o)        -> 3 / 2 / 1 / 0, and a used hint lowers the cap by 1
    computeScore(o)        = max(0, 1000 - 10*alarm - penalties + bonuses)

Penalties fixed by the spec: wrong traversal click **+1 alarm**; wrong
rotation **+2 alarm and -50 points**; **invalid Act 4 swap +1 alarm**.

**Optimal step counts are computed by the model, never typed into JSON.**

### `LevelScreenBase` (`include/screens/LevelScreenBase.h`) — reuse this

Partner A's base class already gives you: the briefing dialog, the canvas
with camera and animation, the HUD with the alarm meter, the toolbar, the
event log panel, hints, undo, replay with a scrubber, the win/lose flow,
and the call into `app.levelFinished`. It holds an `AVLTree`, so it suits
Act 1 directly if you treat that tree as a plain binary tree. For Act 4
you will probably want a sibling base class of your own that swaps the
tree for a `MaxHeap` plus `ArrayView` but keeps the same HUD and toolbar
widgets.

Services available to a subclass: `setObjective`, `showTreeStatic`,
`beginOperation`, `endOperation`, `countVisits`, `addAlarm`, `penalty`,
`win`, `lose`, `saveUndo`, `restoreUndo`, `fitCameraToTree`,
`restartLevel`, `openCustomDialog`, `makeOutcome`, `flash`, `worldMouse`.

Hooks you override: `onLevelStart()` (pure virtual — build the structure,
set the objective and `optimalSteps`), `onNodeClicked`, `onLevelEvent`,
`onLevelUpdate`, `onAnimationDone`, `onUndoRestored`, `onDialogChoice`,
`drawCanvasOverlay`, `drawUiOverlay`, `allowUndo`.

### Widgets you get for free (`include/gui/widgets/`)
`Hud` (alarm meter, score, objective, steps), `Toolbar`, `EventLogPanel`,
`OperationPanel`, `Button`, `RoundedRect`, `Tooltip`, `Dialog`,
`HintPanel`, `Slider`, `TextField`. **Use them; do not write your own.**

### `LayoutEngine` — your heap layout already exists

    int layoutCompleteTree(int n, NodePlacement out[]);   // for Act 4 heaps

Partner A already wrote it. `ArrayView` is the piece you add: the row of
array cells under the tree, showing index numbers, with the cell and the
tree node for the same index highlighted together.

---

## 8. Levels: how they are defined and loaded

`assets/levels/levels.txt` lists level files in campaign order, and `#`
starts a comment. Yours are the commented-out Act 1 and Act 4 sections:

    # Act 1 (Partner B)
    # assets/levels/act1/level_1_1.json
    # Act 2
    assets/levels/act2/level_2_1.json
    ...
    # Act 4 (Partner B)

Each level file is **one flat JSON object**, read by the project's own
`LevelLoader` (no JSON library). It supports only string values, integer
values, and arrays of integers or strings — keep to that. A real example:

    { "id": "2.1", "act": 2, "title": "Search Heist",
      "mission": "Find the marked treasure room",
      "concept": "BST search",
      "briefing": "The vault sorts its rooms: smaller keys lie left, larger keys lie right...",
      "type": "bst_search", "seed": 2101,
      "generator": "random", "n": 12, "keyMin": 10, "keyMax": 99, "keys": [],
      "k": 0, "lo": 0, "hi": 0,
      "hints": ["Compare the target with the current room: smaller goes left, larger goes right.",
                "The gold diamond marks the target room."] }

`LevelDef` (`include/game/LevelDef.h`) holds exactly these fields in
fixed-size arrays. `generator` is `"random"`, `"sorted"` or `"explicit"`,
where `"explicit"` uses `keys`. **Convention:** the first `n` entries of
`keys` build the starting structure, and any entries after that are
scripted later inserts — Act 3 uses this for its rounds, and Act 4 can use
it for scripted deliveries.

`type` is the string your `LevelFactoryB` switches on. Pick clear names in
the same style as A's (`bst_search`, `avl_single_rot`), for example
`bt_traversal`, `bt_build`, `bt_lca`, `bt_mirror`, `bt_boss`,
`heap_build`, `heap_insert`, `heap_extract`, `heap_sort`, `heap_boss`.

Loading a level is two calls — copy from `src/screens/DevMenuScreen.cpp`:

    LevelDef def;
    if (LevelLoader::load(path, def))
    {
        Screen* screen = LevelFactory::create(app, def);
        if (screen != nullptr)
        {
            app.push(screen);
        }
    }

**`Limits` caps everything** (`include/core/Limits.h`):
`MAX_NODES_GUI = 256`, `MAX_EVENTS = 4000`, `MAX_SNAPSHOTS = 400`,
`MAX_LEVEL_KEYS = 64`, `MAX_HINTS = 5`, `MAX_SCREENS = 10`. Respect them —
`MAX_SCREENS = 10` in particular means your menu stack must pop, not grow.

---

## 9. Suggested build order (the deadline is tomorrow — follow this)

Commit after each phase. If you run out of time, the earlier phases alone
are still a submittable, demonstrable half.

**Phase B1 — model plus tests (highest marks per hour, no GUI risk).**
`MaxHeap`, `HeapValidator`, `BinaryTree`, plus `tests/test_heap.cpp` and
`tests/test_binarytree.cpp`. Register them in `tests/main.cpp` the way the
existing ones are: declare the `void runXTests(TestRunner& t);` prototype
at the top and call it in `main`. These are pure C++, testable with no
window, and they are what the viva will actually dig into.

**Phase B2 — benchmarks.** Heap construction (`buildMaxHeap` at O(n)
versus n separate inserts at O(n log n)), Heap Sort, and a sorting
comparison. Add them to `bench/main.cpp` with `setEventLog(nullptr)` and
`sf::Clock` for timing. Write CSV into `bench/results/`; the report charts
are made in Excel.

**Phase B3 — Act 4 GUI.** `ArrayView`, then the heap levels. Act 4 is the
spec's showcase act — the array and the tree view of the same heap, side by
side, highlighted in step — and it is worth more visually than Act 1.

**Phase B4 — Act 1 GUI.** The traversal levels. `LevelScreenBase` does
most of the work.

**Phase B5 — shell screens.** Main menu, campaign map, result screen,
settings, leaderboard, `SaveManager`. Then rewrite `apps/main.cpp` to open
the main menu instead of A's dev menu, and upgrade `App::levelFinished` to
push your Result screen.

**Phase B6 — endless mode, sandbox Binary and Heap tabs, docs**
(`README.md`, `docs/LEVEL_GUIDE.md`, heap report sections).

---

## 10. UI design — it must match Partner A's half exactly

The theme is a **dark fantasy vault heist**, and it already looks premium.
Every colour and size is a `static` function in `class Theme`
(`include/gui/Theme.h`). **Use only those — never a hard-coded colour.**

Background `#0B0F17` with a vignette and a blueprint grid; panels
`#151C2A` with a 1px `#2A3448` border and 12px rounded corners; text
`#E6E9EF` and dim `#8A93A6`; balanced green `#3FB97A`; warning yellow
`#EAD94C`; violation red `#E5484D`; current-visit blue `#4C9AFF`; pruned
grey `#4A5263`; target gold `#D4A537`. The alarm meter blends amber into
red and pulses when nearly full.

**Never use colour as the only signal** — this is an accessibility
requirement in the spec. Violations also get a `!` and a thick outline, the
target gets a double ring and a diamond marker, pruned parts are greyed
*and* labelled `pruned`, and the current room gets a pulsing ring.

Rooms are circular vault doors: metallic ring, darker disc, key in the
centre, badge beside it, hover glow. Corridors are 3px lines that light up
as the path is visited. Every change animates with the ease-in-out curve
`t*t*(3-2*t)` (`Theme::ease`); one event takes 350ms at 1x; the speed
slider runs 0.25x to 4x; inserts grow in, removals fade, rotations slide,
and wins burst gold particles (`ParticleBurst`).

The layout is 1600x900, resizable, VSync on, anti-aliasing 8: canvas on
the left at about 70% width, right column holding HUD then Event Log then
Operation Panel, Toolbar along the bottom, hint panel sliding in. A
200-node structure must hold 60fps, so put edges in one `sf::VertexArray`
and reuse shapes instead of constructing them per frame.

Controls, consistent across both halves: click to select or answer; hover
for a tooltip; drag a child onto its parent to rotate (Act 3); wheel zooms
at the cursor; drag the background to pan; **F** fit; **Space**
play/pause; **left/right arrows** step; **Z** undo; **H** hint; **Esc**
back; **F12** screenshot into `screenshots/`.

Undo takes a snapshot before each player action — `copyFrom` for trees,
`MaxHeap::load` for heaps. **Z** restores it and snaps the layout.

---

## 11. Bugs Partner A already hit — do not repeat them

These cost real debugging time. All of them are fixed in A's code, but the
same mistakes are easy to make again in yours.

1. **Uninitialised `seen[]` in the animator.** `applyShape` only cleared
   the flags for slots that already existed, so every newly added node read
   uninitialised stack memory and was faded out and deleted — levels looked
   completely empty. **Lesson: initialise every element of a fixed array
   over its whole declared length, not just the part you think is live.**

2. **`window.getDefaultView()` is frozen at window-creation size**, so the
   UI was cut off on the right whenever the window was maximised. A added
   `App::useUiView()`, which builds a 1:1 pixel view from the window's
   *current* size. **Call `app.useUiView()` before drawing panels and text;
   never use `getDefaultView()`.**

3. **A dialog opened during `onEnter` is swallowed by the briefing dialog.**
   If one of your levels asks a question as it starts, park the request and
   replay it once the briefing closes — `LevelScreenBase` already has that
   pending-dialog mechanism, so use it rather than opening a dialog
   directly from `onLevelStart`.

4. **A one-shot "animation finished" signal consumed in the wrong state**
   froze level 2.5 forever. If one of your phases chains off the animation
   ending, gate it on `state == ST_PLAY`.

5. **Fixed-width dialogs overflow with many buttons.** `Dialog::layout`
   now sizes the panel to its button row. If you add a widget with a
   variable number of children, size the container from its content.

---

## 12. Git workflow

- Your branch is **`ali`**. Partner A's is `awais`, and `main` holds A's
  finished work.
- Run `git pull --rebase origin main` before you push, so A's fixes come
  along.
- Commit after every working step with clear messages, for example
  `feat(heap): siftDown with comparison counters` or
  `fix(act4): invalid swap penalty`.
- Push to `ali` after every phase, then open a pull request into `main`.
- **Never force-push to `main`.**
- `.gitignore` already covers `build/`, `.vs/`, `.vscode/`,
  `screenshots/*.png`, `bench/results/*.csv`, `*.zip`.
- **Write commit messages with the Bash tool and a heredoc**
  (`git commit -F - << 'MSG'`). PowerShell here-strings mangle parentheses
  and `<`, which once silently committed the wrong set of files.

---

## 13. Definition of done (Partner B)

- [ ] `BinaryTree`, `MaxHeap` and `HeapValidator` fully implemented, every
      stub body replaced, no `// TODO(B)` left in them.
- [ ] `tests/test_binarytree.cpp`, `test_heap.cpp` and `test_save.cpp`
      written, registered in `tests/main.cpp`, and passing — **and A's 173
      checks still passing**.
- [ ] Both recursive and iterative versions of all three DFS traversals,
      plus level-order, verified against hand-worked examples.
- [ ] A seeded random test of 10,000 heap operations with
      `HeapValidator::checkHeap` after every step.
- [ ] Benchmarks write CSV, and heap construction, Heap Sort and the
      sorting comparison are tabulated with a plain-English explanation.
- [ ] Act 1 and Act 4 levels can each be **completed, failed and retried**.
- [ ] Sandbox Binary and Heap tabs offer every operation.
- [ ] Main menu, campaign map, result screen, settings and leaderboard
      work; `apps/main.cpp` opens the main menu; progress saves and loads.
- [ ] A grep confirms none of the §2 forbidden items appear: no STL
      container, nothing from `<algorithm>`, no `enum`, no global variable,
      no `auto`, lambda or template.
- [ ] `README.md` and `docs/LEVEL_GUIDE.md` written.
- [ ] Everything pushed to `ali` with a pull request into `main`.

---

## 14. How to work with the user

- They are a second-year student submitting this for marks tomorrow, and
  they will have to **explain every line in a viva**. Simple and
  explainable beats clever, every time.
- They want the UI to look genuinely impressive, and A's half already does,
  so yours must not look like a different game.
- They tend to report bugs from gameplay screenshots, in the form "nothing
  happens when I click". **Actually build and run the game and play your
  own levels before telling them a level works.** `./build.ps1 run`, and
  F12 saves a screenshot you can then look at.
- When something is broken, fix the root cause rather than the symptom.
  All of the bugs in §11 presented as several unrelated glitches and each
  turned out to be a single defect.

# CLAUDE.md — Vault Heist: The Living Vault (Data Structures Project)

> **Current developer: Partner A.** (Partner B uses this same file and changes only this line.)
> The full official spec is `docs/Project_Deliverable_1.pdf`. Read it fully before any work. If this file and the spec disagree, the spec wins — tell me about the conflict.
> **Deadline: 20 Nov 2026, 11:59 PM.** Working, simple, tested code beats clever code.

---

## 0. MOST IMPORTANT RULE: the official constraints + simple, readable C++

These are the **official course constraints**. Breaking any of them costs marks.

| Rule | Detail |
|---|---|
| ❌ **No STL** | No `std::vector`, `std::list`, `std::map`, `std::set`, `std::unordered_map`, `std::stack`, `std::queue`, `std::deque`, `std::priority_queue`, `std::array`, `std::pair`, `std::tuple`, and nothing from `<algorithm>` (`sort`, `swap`, `max`, `make_heap`…). We implement every data structure and helper ourselves. |
| ✅ `std::string` | Allowed for text. |
| ✅ SFML types | `sf::Vector2f`, `sf::Text`, `sf::Color`, `sf::Event`, `sf::Clock`, `sf::VertexArray`, etc. are allowed for GUI and timing. We must be able to explain each one we use. |
| ✅ Standard C libraries | `<cmath>`, `<cstdlib>` are allowed, along with `<iostream>`, `<fstream>`, `<sstream>` for console and file I/O. |
| ✅ `using namespace std;` | Allowed. |
| ❌ **No global variables** | Every variable lives inside a function or a class. No global counters, no global constants, no global objects. |
| ❌ **No enums** | Neither `enum` nor `enum class`, anywhere. Use named integer constants **inside a class** instead: `static const int VISIT = 0;` and store the type as `int`. (Using SFML's own built-in values like `sf::Keyboard::Z` is fine; we just never declare an enum ourselves.) |
| ✅ Leaf height = 1 | Empty tree = 0. |

**Also keep the code simple.** We are second-year students and must explain every line in a viva. Don't use:
- templates, lambdas or `auto`
- `std::optional`, `std::function`, smart pointers
- move semantics, operator overloading, exceptions
- `std::filesystem`, `<chrono>`, `<random>`
- any library other than SFML: no JSON library, no test framework, no Python

**Formatting and comments**
- Consistent formatting everywhere:
  - 4-space indentation
  - opening brace on its own line (Allman style)
  - one statement per line, a space after `if`/`for`/`while`
  - blank lines between functions
  - private members at the bottom of the class
- Clear English names (`insertKey`, `rotateRight`, `alarmSteps`). One class per `.h`/`.cpp` pair.
- **Comments only where they add value.** Use a short comment above a non-obvious function, and an inline comment on tricky lines (rotation pointer changes, successor logic, heap index maths, layout maths). Don't comment obvious code (`// increment i`), don't write banner comments, and don't leave commented-out code.
- Keep functions short (≈ 40 lines max); split into helpers if longer.
- Return "not found" as a `bool` with an `int&` out-parameter, e.g. `bool kth(int k, int& result)`.
- Lists of results go into a caller-provided array plus a returned count, e.g. `int inorderKeys(int out[], int maxCount)`.
- Always free memory you `new` (destructors, `clear()`).

## 1. What we are building

A C++ SFML game where the map is a tree of vault rooms. Each move costs alarm. The player learns:
- Act 1: general binary trees
- Act 2: BSTs
- Act 3: AVL trees
- Act 4: Max Heap and Heap Sort

Two partners each own specific files (§3). Both share one **contract** (the headers in `include/`), so their code fits together without merge errors.

## 2. Tech stack (fixed)

- **C++17** compiler (MSVC from VS 2022 Build Tools, or MinGW-w64), Windows + VS Code + PowerShell.
- **SFML 2.6.2**, pulled in automatically by a short, well-commented `CMakeLists.txt` (FetchContent, static linking). Also provide `build.ps1` so we can build and run with one command: `./build.ps1` then `./build.ps1 run`, `./build.ps1 test`, `./build.ps1 bench`.
- **Fonts** in `assets/fonts/`, all free OFL, with their licence files:
  - Cinzel for titles
  - Inter for UI text
  - JetBrains Mono for the event log and numbers
- **Level files:** simple JSON read by our **own small reader** (`LevelLoader`). It only needs to support flat objects with numbers, strings and arrays of numbers/strings. Keep it simple and commented.
- **Timing** for benchmarks: `sf::Clock` (SFML).
- **Charts** for the report: benchmarks write `.csv` files that we open in Excel to make charts. No Python.

## 3. Ownership (who writes what)

| Area | Partner A | Partner B |
|---|---|---|
| Model | `BST`, `AVLTree`, tree validation (`TreeValidator`) | `BinaryTree` (Act 1 API), `MaxHeap` + heap sort, `HeapValidator` |
| Shared core (A writes once in Phase 0, complete) | `Limits.h`, `Event.h`, `EventLog`, `TreeNode.h`, `NodeStack`, `NodeQueue`, `Random`, `ScoreSystem`, `LevelDef.h`, `LevelLoader`, `Screen.h`, `App`, `Theme.h`, CMake + `build.ps1` | Extends `App` (menus etc.) after Phase 0 |
| Model | `BST`, `AVLTree`, `TreeValidator` | `BinaryTree`, `MaxHeap`, `HeapValidator` |
| GUI engine | `LayoutEngine` (tree layout + complete-tree layout for heaps), `TreeRenderer`, `AnimationController`, `Camera` (zoom/pan/fit), tooltips | `ArrayView` (Act 4 array cells, tree/array sync) |
| In-level widgets (B reuses them) | `Hud` (alarm meter, score, objective, steps), `Toolbar`, `EventLogPanel`, `OperationPanel`, `Button`, `RoundedRect`, `Tooltip`, `Dialog`, `HintPanel` | — |
| Screens | Act 2 (2.1–2.5), Act 3 (3.1–3.5), Sandbox BST/AVL tabs, dev launcher `apps/dev_a.cpp` | Main menu, Campaign map, Result screen, Settings, Leaderboard, Act 1 + Act 4 levels, Endless, Sandbox Binary/Heap tabs, final `apps/main.cpp` |
| Level creation | `LevelFactoryA.cpp` + Act 2–3 JSON | `LevelFactoryB.cpp` + Act 1/4 JSON, `SaveManager`, `LEVEL_GUIDE.md` |
| Tests | `tests/test_bst.cpp`, `test_avl.cpp`, `test_tree_validator.cpp`, `test_tree_events.cpp` | `test_binarytree.cpp`, `test_heap.cpp`, `test_save.cpp` |
| Benchmarks (spec §10) | Height comparison, search cost, rotation count, order statistics | Heap construction, Heap Sort, sorting comparison |
| Docs | `docs/VIVA_NOTES_A.md`, `docs/TEST_REPORT_A.md`, `docs/BENCH_RESULTS_A.md`, `docs/REPORT_A.md` | README, LEVEL_GUIDE, heap report sections, final report |

**Rules**
- Only edit files you own.
- In Phase 0, Partner A creates **compiling stubs** for every Partner B file. Stub bodies return 0/false/do nothing and are marked `// TODO(B)`. After Phase 0, do not touch B's files.
- If a shared header in `include/core`, `include/model`, `include/game` or `include/gui/Theme.h`/`Screen.h`/`App.h` needs a change, **stop and ask me first**, because my partner must agree to it.

## 4. Project rules

1. **No library trees or heaps** (see §0). Our own `NodeStack` and `NodeQueue` (simple arrays of `TreeNode*` that grow with `new`) are used for iterative traversals.
2. **Model code never includes SFML.** Files in `src/model` and `include/model` are pure C++ so they can be unit-tested alone.
3. **Every model operation writes events** to an `EventLog*` given with `setEventLog(log)`. If the pointer is `nullptr` (benchmarks), no events are written. The GUI only plays events back. It never guesses what happened.
4. **Deep-path safety.** A BST built from sorted keys has height n (10,000 in benchmarks), and Windows has a small stack, so deep recursion crashes.
   - Every **BST** function that walks down or up a path must use **loops, not recursion**: insert, search, delete, min/max, successor/predecessor, height/size update, range query, inorder, destructor/clear, copy.
   - **AVL** may use recursion, because its height is O(log n).
5. **Validation after every operation** in tests, and in the GUI when the `App` member `bool debugValidate` is true.
6. **Unique node ids.** Each tree has its own `int nextId` member counter, with no global counter. Ids are never reused and stay the same across rotations, copies (`copyFrom` also copies `nextId`) and value swaps. The GUI tracks nodes by id.
7. **Leaf height = 1** (official rule). Empty = 0, so sorted inserts of n keys give height n (spec 2.5). The balance factor uses the same heights.
8. **Seeded randomness only**, using our own `Random` class: a small linear congruential generator, created as a local object or class member and seeded from the level `seed`. Never use `rand()` or the current time.
9. **Optimal steps are computed by the model**, never typed into JSON.
10. **Duplicates are rejected**, and the GUI shows a message.
11. **Build and run tests after every change.** Never commit code that doesn't compile or fails tests.
12. If the spec is unclear, choose the simplest sensible interpretation and write it in `docs/DESIGN_DECISIONS.md`.

## 5. The shared contract (Phase 0 creates these exactly)

See the actual headers in `include/` — they are the contract. Summary:

- `include/core/Limits.h` — all fixed sizes (`MAX_NODES_GUI`, `MAX_EVENTS`, `MAX_SNAPSHOTS`, `MAX_LEVEL_KEYS`, `MAX_HINTS`, `MAX_SCREENS`).
- `include/core/Event.h` — `struct NodeState` and `class Event` with the event-type constants (`VISIT`…`DONE`), side/cmp/rotation/highlight constants, and all payload fields.
- `include/core/EventLog.h` — events + preorder tree-shape snapshots + array snapshots for one operation. Tree snapshots are saved after INSERT, UNLINK, SWAP_VALUE and ROTATE; the renderer re-lays out the shape and slides nodes by id. Every batch ends with a DONE event.
- `include/model/TreeNode.h` — id, key, treasure, height (leaf = 1), size, left/right/parent.
- `include/model/BST.h` (A) — iterative insert/search/remove (`DEL_` case constants), min/max, successor/predecessor, pruned rangeQuery, kthSmallest, lcaBST, copyFrom (keeps ids + nextId), searchPathLength, inorderKeys, counters.
- `include/model/AVLTree.h` (A) — extends BST; auto/manual rebalance, rotateLeftAt/rotateRightAt, rebalanceAt/caseAt/predictInsert (ROT_ constants), balanceFactor, kth/rank/countInRange via sizes, isBalanced, unbalancedIds, rotation counters.
- `include/model/BinaryTree.h` (B) — Act 1 API: recursive + iterative traversals, buildPreIn/buildPostIn, height, diameter, lca, pathTo, mirror, isSameTree, isSymmetric.
- `include/model/MaxHeap.h` (B) — array heap: parentOf/leftOf/rightOf, buildMaxHeap, siftUp/siftDown, heapify, extractMax, insert, heapSort, playerSwap, isMaxHeap, load, counters.
- Validators — `TreeValidator` (A): checkStructure / checkBST / checkAVL as static functions. `HeapValidator` (B): checkHeap.
- `ScoreSystem` — `LevelOutcome` struct; `alarmCapacity(optimal) = ceil(1.5*optimal)+2`; stars 3/2/1/0 with the hint cap; `score = max(0, 1000 - 10*alarm - penalties + bonuses)`. Penalties: wrong traversal click +1 alarm; wrong rotation +2 alarm, -50 points; invalid Act 4 swap +1 alarm.
- Levels — `assets/levels/levels.txt` lists the files in campaign order; each level JSON is flat; `LevelDef` holds the fields in fixed arrays; `LevelLoader::load` is static; `LevelFactory::create` tries `LevelFactoryA` then `LevelFactoryB` with plain if-chains.
- GUI — `Screen` base class (onEnter / handleEvent / update / draw, virtual destructor); `App` holds the window, three fonts, a screen stack, `debugValidate` and `levelFinished` (Phase 0: print and pop; B upgrades it to the Result screen). The `App` object is a local in `main()`.

## 6. UI design (must look premium and consistent across both partners)

The theme is **a dark fantasy vault heist**. All colours and sizes live in `class Theme` (`include/gui/Theme.h`) as **static functions**. Both partners use only these.

Palette: background `#0B0F17` with vignette + blueprint grid; panels `#151C2A` with 1 px `#2A3448` border and 12 px rounded corners; text `#E6E9EF` / dim `#8A93A6`; balanced green `#3FB97A`; warning yellow `#EAD94C`; violation red `#E5484D`; current visit blue `#4C9AFF`; pruned grey `#4A5263`; target gold `#D4A537`; the alarm meter blends amber → red and pulses when nearly full.

**Never use colour alone**: violations also get a "!" and a thick outline; the target a double ring and a marker; pruned parts are greyed and labelled "pruned"; the current room gets a pulsing ring.

Rooms are circular vault doors (metallic ring, darker disc, key in the centre, badge, hover glow). Corridors are 3 px lines that light up as the path is visited. Every change animates with ease-in-out `t*t*(3-2*t)`; each event takes 350 ms at 1×; speed slider 0.25×–4×; insert grows in, remove fades, rotations slide, wins burst gold particles.

Layout 1600×900 (resizable, VSync, AA 8): canvas left ~70%; right column = HUD, Event Log, Operation Panel; Toolbar at the bottom; hint panel slides in. A 200-node tree must stay at 60 fps (edges in one `sf::VertexArray`, reused shapes).

Controls: click select/answer; hover tooltip; drag child onto parent = rotate; wheel zoom at cursor; drag background pan; F fit; Space play/pause; ←/→ step; Z undo; H hint; Esc back; F12 screenshot to `screenshots/`.

Undo: snapshot (`copyFrom` / `load`) before each player action; Z restores and snaps the layout. Replay: re-runs the last `EventLog` with a scrubber bar.

## 7. Partner A level designs (Acts 2–3)

Each VISIT adds 1 alarm, and capacity = `alarmCapacity(optimal)`.
- **2.1 Search Heist.** The target key is shown; click the correct child at each room. Optimal = `searchPathLength(target)`.
- **2.2 Stash Loot.** Walk the insert path; the new room grows in at the correct leaf. Wrong turn +1.
- **2.3 Steal and Collapse.** Pick the case (Leaf / One child / Two children), then watch the delete; the two-child case highlights the successor and animates the value swap. Several deletions cover all cases.
- **2.4 Range Raid** `[lo, hi]`. Collect every key in range; afterwards pruned parts are greyed. Optimal = rooms the pruned query visits; perfect pruning earns a bonus.
- **2.5 Boss: The Lazy Architect.** Sorted keys build a corridor; search a deep key while capacity shrinks; ends with the "why balancing matters" cutscene panel.
- **3.1 Read the Badges.** Height badges only; click every |BF| > 1 room, Submit, then the coloured badges are revealed. Built with auto-rebalance off.
- **3.2 Single rotations.** Drag a child onto its parent; wrong choice +2 alarm, −50. LL and RR each appear at least twice.
- **3.3 Double rotations.** Name the case (LR/RL) first, then perform the two rotations in order.
- **3.4 Auto mode and prediction.** Pick None/LL/RR/LR/RL before each insert; checked against `predictInsert`.
- **3.5 Boss: Order-statistic heist.** Steal the k-th most valuable = (n−k+1)-th smallest using size labels. Optimal = path length of `kth`. Rotations happen between rounds; the player confirms the changed size labels; wrong sizes are flagged.
- Act 3 levels make **each rotation type appear at least twice**.

## 8. Repo layout
```
CMakeLists.txt  build.ps1  CLAUDE.md  README.md  .gitignore
docs/  assets/fonts/  assets/levels/
include/core  include/model  include/game  include/gui  include/gui/widgets  include/screens
src/core  src/model  src/game  src/gui  src/gui/widgets  src/screens
tests/  bench/  apps/main.cpp  apps/dev_a.cpp  screenshots/
```
Executables: `vault_heist` (final game, B owns main.cpp), `dev_a` (A's launcher), `run_tests`, `run_bench` (Release, EventLog = nullptr).

## 9. Testing (no framework)
- `class TestRunner` has `int passed, failed;` and `void check(bool, const string&)` printing `[PASS]`/`[FAIL]`.
- Each test file has one function taking the runner by reference; `tests/main.cpp` calls them all; exit code 1 if anything fails; output saved to `docs/test_output_A.txt`.

## 10. Git workflow
- Phase 0 (contract + stubs) goes to `main`. After that, Partner A works on branch **`partner-a`**.
- Commit after each working step, with clear messages (`feat(avl): LR and RL rotations`).
- Run `git pull --rebase origin main` before pushing, and push after every phase.
- `.gitignore`: `build/`, `.vs/`, `.vscode/`, `screenshots/*.png`, `bench/results/`.
- Never force-push to `main`.

## 11. Definition of done (Partner A)
- All A tests pass, covering every tree case in spec §9.1; a 10,000-operation seeded random test with the validator after each step passes.
- Benchmarks write CSV files; `docs/BENCH_RESULTS_A.md` has the tables plus a plain-English explanation.
- Levels 2.1–2.5 and 3.1–3.5 can each be completed, failed and retried.
- The Sandbox BST/AVL tabs offer every operation.
- Screenshots exist of all 4 AVL rotations.
- A search confirms none of the forbidden items from §0 are used, including no `enum` and no global variables.
- `docs/VIVA_NOTES_A.md` explains every algorithm in simple words with worked examples, complexity, and likely viva questions.
- Everything is pushed to `partner-a`, with a pull request to `main` opened.

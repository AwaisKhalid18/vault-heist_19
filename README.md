# Vault Heist: The Living Vault

A C++17 / SFML 2.6.2 puzzle-strategy game for the Data Structures project.
The vault is a tree of rooms: the player learns binary trees, BSTs, AVL
rotations and heaps by breaking in. Every data structure is written from
scratch (no STL containers) and is fully separated from the GUI through an
event log, so every operation can be unit-tested and replayed as animation.

## Build and run (Windows, PowerShell)

Requirements: **Git**, **Visual Studio 2022+ Build Tools** with the
"Desktop development with C++" workload (CMake and Ninja are included).
SFML 2.6.2 is downloaded and built automatically on the first configure.

```powershell
./build.ps1          # configure + build (Release)
./build.ps1 run      # start the game
./build.ps1 dev      # start Partner A's dev launcher (levels, sandbox, demos)
./build.ps1 test     # run the unit tests (output saved to docs/test_output_A.txt)
./build.ps1 bench    # run the benchmarks (CSV written to bench/results/)
./build.ps1 clean    # delete the build folder
```

The first build takes a few minutes while SFML compiles. After that,
builds are incremental and fast.

## Game controls

| Input | Action |
|---|---|
| Click a room | Select / answer / move |
| Hover a room | Tooltip with key, height, balance factor, size, depth |
| Drag a child room onto its parent | Manual AVL rotation (Act 3) |
| Mouse wheel | Zoom at the cursor |
| Drag the background | Pan |
| F | Fit the tree to the view |
| Space | Play / pause the animation |
| Left / Right | Step through events |
| Z | Undo |
| H | Hint (costs one star) |
| Esc | Back |
| F12 | Save a screenshot PNG into `screenshots/` |

In the dev launcher (`./build.ps1 dev`), keys **1-4** open the LL / RR /
LR / RL rotation demos used for the report screenshots.

## Project structure

```
include/, src/        core (events, log), model (trees, heap), game, gui, screens
apps/                 main.cpp (final game), dev_a.cpp (Partner A launcher)
tests/                our own tiny test runner + the unit tests
bench/                benchmarks; results land in bench/results/*.csv
assets/levels/        level JSON files + levels.txt (campaign order)
assets/fonts/         Cinzel, Inter, JetBrains Mono (all OFL, licences included)
docs/                 reports, viva notes, design decisions
```

## Ownership

- **Partner A (Awais):** shared core and contract, BST, AVL, TreeValidator,
  the GUI engine (layout, renderer, animation, camera, widgets), the
  Sandbox BST/AVL tabs, Act 2 and Act 3 levels, tree benchmarks and docs.
- **Partner B:** BinaryTree (Act 1), MaxHeap + heap sort (Act 4),
  HeapValidator, ArrayView, menus/result/settings screens, SaveManager,
  Act 1/4 levels, endless mode, heap benchmarks. Partner B's files
  currently contain compiling stubs marked `// TODO(B)`.

## Adding a level

Drop a JSON file into `assets/levels/act*/` and add its path to
`assets/levels/levels.txt`. No code change is needed; the format is
documented in `CLAUDE.md` section 5 (Partner B writes the full
LEVEL_GUIDE.md).

// Partner B's levels: Act 1 (general binary trees) and Act 4 (Max Heap and
// Heap Sort). A plain if-chain on the level's `type` string, matching
// LevelFactoryA - no maps and no function pointers.
#include "game/LevelFactory.h"
#include "screens/Level11Traversal.h"
#include "screens/Level12Rebuild.h"
#include "screens/Level13Heights.h"
#include "screens/Level14Lca.h"
#include "screens/Level15Mirror.h"
#include "screens/Level41Build.h"
#include "screens/Level43Extract.h"
#include "screens/Level45Boss.h"

Screen* LevelFactoryB::create(App& app, const LevelDef& def)
{
    // ---- Act 1: the Blueprint Wing ----
    if (def.type == "bt_traversal")
    {
        return new Level11Traversal(app, def);
    }
    if (def.type == "bt_build")
    {
        return new Level12Rebuild(app, def);
    }
    if (def.type == "bt_height")
    {
        return new Level13Heights(app, def);
    }
    if (def.type == "bt_lca")
    {
        return new Level14Lca(app, def);
    }
    if (def.type == "bt_mirror")
    {
        return new Level15Mirror(app, def);
    }

    // ---- Act 4: the Heap Fortress ----
    // 4.1 and 4.2 share one screen because the player's move is the same in
    // both; only the starting array differs, and that comes from the JSON.
    if (def.type == "heap_build")
    {
        return new Level41Build(app, def);
    }
    if (def.type == "heap_repair")
    {
        return new Level41Build(app, def);
    }
    // 4.3 takes the k largest treasures, 4.4 takes all of them.
    if (def.type == "heap_extract")
    {
        return new Level43Extract(app, def);
    }
    if (def.type == "heap_race")
    {
        return new Level43Extract(app, def);
    }
    if (def.type == "heap_boss")
    {
        return new Level45Boss(app, def);
    }

    return nullptr;
}

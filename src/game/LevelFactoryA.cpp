#include "game/LevelFactory.h"
#include "screens/Level21Search.h"
#include "screens/Level22Insert.h"
#include "screens/Level23Delete.h"
#include "screens/Level24Range.h"
#include "screens/Level25Boss.h"
#include "screens/Level31Badges.h"
#include "screens/Level32SingleRot.h"
#include "screens/Level33DoubleRot.h"
#include "screens/Level34Predict.h"
#include "screens/Level35BossKth.h"

// Partner A owns the Act 2 and Act 3 level types. Plain if-chain.
Screen* LevelFactoryA::create(App& app, const LevelDef& def)
{
    if (def.type == "bst_search")
    {
        return new Level21Search(app, def);
    }
    if (def.type == "bst_insert")
    {
        return new Level22Insert(app, def);
    }
    if (def.type == "bst_delete")
    {
        return new Level23Delete(app, def);
    }
    if (def.type == "bst_range")
    {
        return new Level24Range(app, def);
    }
    if (def.type == "bst_boss")
    {
        return new Level25Boss(app, def);
    }
    if (def.type == "avl_badges")
    {
        return new Level31Badges(app, def);
    }
    if (def.type == "avl_single_rot")
    {
        return new Level32SingleRot(app, def);
    }
    if (def.type == "avl_double_rot")
    {
        return new Level33DoubleRot(app, def);
    }
    if (def.type == "avl_predict")
    {
        return new Level34Predict(app, def);
    }
    if (def.type == "avl_boss_kth")
    {
        return new Level35BossKth(app, def);
    }
    return nullptr;
}

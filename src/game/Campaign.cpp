#include "game/Campaign.h"
#include "game/LevelLoader.h"

Campaign::Campaign()
{
    levelCount = 0;
}

bool Campaign::load(const string& listPath)
{
    levelCount = 0;
    string paths[MAX_CAMPAIGN_LEVELS];
    int pathCount = LevelLoader::loadLevelList(listPath, paths, MAX_CAMPAIGN_LEVELS);
    for (int i = 0; i < pathCount; i++)
    {
        LevelDef def;
        if (LevelLoader::load(paths[i], def) && levelCount < MAX_CAMPAIGN_LEVELS)
        {
            defs[levelCount] = def;
            levelCount++;
        }
    }
    return levelCount > 0;
}

int Campaign::count()
{
    return levelCount;
}

const LevelDef& Campaign::get(int index)
{
    if (index < 0)
    {
        index = 0;
    }
    if (index >= levelCount)
    {
        index = levelCount > 0 ? levelCount - 1 : 0;
    }
    return defs[index];
}

int Campaign::indexOfId(const string& levelId)
{
    for (int i = 0; i < levelCount; i++)
    {
        if (defs[i].id == levelId)
        {
            return i;
        }
    }
    return -1;
}

int Campaign::countInAct(int act)
{
    int total = 0;
    for (int i = 0; i < levelCount; i++)
    {
        if (defs[i].act == act)
        {
            total++;
        }
    }
    return total;
}

int Campaign::firstOfAct(int act)
{
    for (int i = 0; i < levelCount; i++)
    {
        if (defs[i].act == act)
        {
            return i;
        }
    }
    return -1;
}

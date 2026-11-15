#include "game/LevelFactory.h"

Screen* LevelFactory::create(App& app, const LevelDef& def)
{
    Screen* screen = LevelFactoryA::create(app, def);
    if (screen != nullptr)
    {
        return screen;
    }
    return LevelFactoryB::create(app, def);
}

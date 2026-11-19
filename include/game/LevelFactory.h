#pragma once

#include "game/LevelDef.h"

class App;
class Screen;

// Partner A's levels (Acts 2 and 3). Returns nullptr for other types.
class LevelFactoryA
{
public:
    static Screen* create(App& app, const LevelDef& def);
};

// Partner B's levels (Acts 1 and 4, endless). Returns nullptr for others.
class LevelFactoryB
{
public:
    static Screen* create(App& app, const LevelDef& def);
};

// Tries A first, then B. A simple if-chain: no maps, no function pointers.
class LevelFactory
{
public:
    static Screen* create(App& app, const LevelDef& def);
};

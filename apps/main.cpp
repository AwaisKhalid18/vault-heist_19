// Vault Heist - the full game.
//
// With no arguments it opens the main menu. Passing a level file opens that
// level straight away, which is how a demo jumps to one act without playing
// through the campaign:
//     ./build/vault_heist.exe assets/levels/act4/level_4_1.json
#include "gui/App.h"
#include "game/LevelLoader.h"
#include "game/LevelFactory.h"
#include "screens/MainMenuScreen.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    App app;
    if (!app.init("Vault Heist"))
    {
        return 1;
    }

    app.save.load(app.savePath);
    app.debugValidate = app.save.validateAfterEachStep;

    if (argc > 1)
    {
        string path = argv[1];
        LevelDef def;
        if (LevelLoader::load(path, def))
        {
            Screen* screen = LevelFactory::create(app, def);
            if (screen != nullptr)
            {
                app.push(screen);
                app.run();
                return 0;
            }
            cout << "No screen is registered for level type " << def.type << endl;
        }
        else
        {
            cout << "Could not load level file " << path << endl;
        }
        return 1;
    }

    app.push(new MainMenuScreen(app));
    app.run();
    return 0;
}

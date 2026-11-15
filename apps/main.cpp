// TODO(B): Partner B owns this file: main menu, campaign map, settings.
// Until then it opens Partner A's dev launcher so the game is runnable.
#include "gui/App.h"
#include "screens/DevMenuScreen.h"

int main()
{
    App app;
    if (!app.init("Vault Heist"))
    {
        return 1;
    }
    app.push(new DevMenuScreen(app));
    app.run();
    return 0;
}

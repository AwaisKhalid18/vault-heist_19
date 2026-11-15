// Partner A's dev launcher: opens the dev menu with direct access to the
// sandbox, every Act 2-3 level, the rotation demos and the stress view.
#include "gui/App.h"
#include "screens/DevMenuScreen.h"

int main()
{
    App app;
    if (!app.init("Vault Heist - Dev A"))
    {
        return 1;
    }
    app.debugValidate = true;   // validate the model after every operation
    app.push(new DevMenuScreen(app));
    app.run();
    return 0;
}

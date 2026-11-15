#pragma once

#include "screens/LevelScreenBase.h"
#include "gui/widgets/Button.h"

// 3.1 Read the Badges: badges show heights only. The player clicks every
// room with |BF| > 1, presses Submit, and the coloured BF badges are
// revealed as the check.
class Level31Badges : public LevelScreenBase
{
public:
    Level31Badges(App& app, const LevelDef& def);

protected:
    void onLevelStart();
    void onNodeClicked(int nodeId);
    void onLevelEvent(const sf::Event& e);
    void onLevelUpdate(float dt);
    void drawUiOverlay(sf::RenderWindow& window);

private:
    Button submitButton;
    bool submitReady;
    int selectedIds[Limits::MAX_NODES_GUI];
    int selectedCount;
    bool revealed;

    bool isSelected(int nodeId);
    void toggle(int nodeId);
    void submit();
};

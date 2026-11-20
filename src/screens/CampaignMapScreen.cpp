#include "screens/CampaignMapScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelFactory.h"
#include "core/TextUtil.h"

CampaignMapScreen::CampaignMapScreen(App& a) : app(a)
{
    mouseX = 0.f;
    mouseY = 0.f;
    ready = false;
    for (int i = 0; i < Campaign::MAX_CAMPAIGN_LEVELS; i++)
    {
        unlocked[i] = false;
    }
}

void CampaignMapScreen::onEnter()
{
    campaign.load("assets/levels/levels.txt");

    title.setFont(app.titleFont);
    title.setCharacterSize(38);
    title.setFillColor(Theme::gold());
    title.setString("CAMPAIGN MAP");

    actTitle.setFont(app.titleFont);
    actTitle.setCharacterSize(Theme::headingSize());
    starLine.setFont(app.monoFont);
    starLine.setCharacterSize(Theme::monoSize());
    footer.setFont(app.uiFont);
    footer.setCharacterSize(Theme::smallSize());
    footer.setFillColor(Theme::skipped());
    footer.setString("A level unlocks once the one before it has been cracked."
                     "   |   Settings can unlock everything for a demo.");

    refreshUnlocks();
    layout();
    ready = true;
}

// Level 1 is always open; after that a level opens once the previous one has
// been cleared, unless the player asked Settings to unlock everything.
void CampaignMapScreen::refreshUnlocks()
{
    bool previousCleared = true;
    for (int i = 0; i < campaign.count(); i++)
    {
        unlocked[i] = app.save.unlockAllLevels || i == 0 || previousCleared;
        previousCleared = app.save.isCleared(campaign.get(i).id);
    }
}

string CampaignMapScreen::actName(int act)
{
    if (act == 1)
    {
        return "ACT 1  The Blueprint Wing";
    }
    if (act == 2)
    {
        return "ACT 2  The Sorted Wing";
    }
    if (act == 3)
    {
        return "ACT 3  The Architect's Wing";
    }
    return "ACT 4  The Heap Fortress";
}

void CampaignMapScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;

    float margin = 24.f;
    float gap = 16.f;
    float columnW = (w - margin * 2.f - gap * 3.f) / 4.f;
    float columnY = 140.f;
    float columnH = h - columnY - 120.f;

    for (int act = 0; act < 4; act++)
    {
        float x = margin + (columnW + gap) * (float)act;
        actPanels[act].setRect(x, columnY, columnW, columnH, Theme::panelRadius());
        actPanels[act].setFillColor(Theme::panel());
        actPanels[act].setOutline(Theme::panelBorder(), 1.f);
    }

    for (int i = 0; i < campaign.count(); i++)
    {
        const LevelDef& def = campaign.get(i);
        int act = def.act >= 1 && def.act <= 4 ? def.act : 1;
        int slot = 0;
        for (int j = 0; j < i; j++)
        {
            if (campaign.get(j).act == def.act)
            {
                slot++;
            }
        }
        float x = margin + (columnW + gap) * (float)(act - 1) + 16.f;
        float y = columnY + 58.f + (float)slot * 76.f;
        levelButtons[i].init(&app.uiFont, def.id + "  " + def.title,
                             x, y, columnW - 32.f, 46.f);
        levelButtons[i].setEnabled(unlocked[i]);
        levelButtons[i].setAccent(unlocked[i] && !app.save.isCleared(def.id));
    }

    backButton.init(&app.uiFont, "Back to menu", margin, h - 76.f, 200.f, 48.f);
}

void CampaignMapScreen::openLevel(int index)
{
    if (index < 0 || index >= campaign.count() || !unlocked[index])
    {
        return;
    }
    Screen* screen = LevelFactory::create(app, campaign.get(index));
    if (screen != nullptr)
    {
        app.push(screen);
    }
}

void CampaignMapScreen::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        mouseX = (float)e.mouseMove.x;
        mouseY = (float)e.mouseMove.y;
    }
    if (e.type == sf::Event::Resized)
    {
        layout();
    }
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
    {
        app.requestPop();
        return;
    }
    if (backButton.handleEvent(e))
    {
        app.requestPop();
        return;
    }
    for (int i = 0; i < campaign.count(); i++)
    {
        if (levelButtons[i].handleEvent(e))
        {
            openLevel(i);
            return;
        }
    }
}

void CampaignMapScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    // returning from a level calls onEnter again, but the hover states still
    // need refreshing every frame
    backButton.update(dt, mouseX, mouseY);
    for (int i = 0; i < campaign.count(); i++)
    {
        levelButtons[i].update(dt, mouseX, mouseY);
    }
}

void CampaignMapScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    sf::Vector2u size = window.getSize();
    float centerX = (float)size.x / 2.f;

    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(centerX - tb.width / 2.f - tb.left, 56.f);
    window.draw(title);

    for (int act = 0; act < 4; act++)
    {
        actPanels[act].draw(window);
        actTitle.setString(actName(act + 1));
        actTitle.setCharacterSize(15);
        actTitle.setFillColor(Theme::gold());
        actTitle.setPosition(actPanels[act].left() + 16.f, actPanels[act].top() + 16.f);
        window.draw(actTitle);
    }

    for (int i = 0; i < campaign.count(); i++)
    {
        levelButtons[i].draw(window);

        const LevelDef& def = campaign.get(i);
        int stars = app.save.starsFor(def.id);
        string line = "";
        for (int s = 0; s < 3; s++)
        {
            line = line + (s < stars ? "*" : "-");
        }
        if (!unlocked[i])
        {
            line = "locked";       // a word as well as the greyed-out button
        }
        else if (stars == 0)
        {
            line = line + "  not cracked";
        }
        else
        {
            line = line + "  best " + TextUtil::intToText(app.save.bestScoreFor(def.id));
        }
        starLine.setString(line);
        starLine.setFillColor(!unlocked[i] ? Theme::skipped()
                              : (stars > 0 ? Theme::gold() : Theme::dimText()));
        // the label sits just under its own button
        float bx = 0.f;
        float by = 0.f;
        for (int act = 0; act < 4; act++)
        {
            if (def.act == act + 1)
            {
                bx = actPanels[act].left() + 16.f;
            }
        }
        int slot = 0;
        for (int j = 0; j < i; j++)
        {
            if (campaign.get(j).act == def.act)
            {
                slot++;
            }
        }
        by = actPanels[0].top() + 58.f + (float)slot * 76.f + 48.f;
        starLine.setPosition(bx, by);
        window.draw(starLine);
    }

    backButton.draw(window);
    sf::FloatRect fb = footer.getLocalBounds();
    footer.setPosition(centerX - fb.width / 2.f - fb.left, (float)size.y - 40.f);
    window.draw(footer);
}

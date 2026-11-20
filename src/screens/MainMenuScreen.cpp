#include "screens/MainMenuScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "screens/CampaignMapScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/LeaderboardScreen.h"
#include "screens/EndlessScreen.h"
#include "screens/SandboxBScreen.h"
#include "core/TextUtil.h"
#include <cmath>

MainMenuScreen::MainMenuScreen(App& a) : app(a)
{
    mouseX = 0.f;
    mouseY = 0.f;
    glowClock = 0.f;
    ready = false;
}

void MainMenuScreen::onEnter()
{
    // the save file is read once here, so every menu below sees the progress
    app.save.load(app.savePath);
    app.debugValidate = app.save.validateAfterEachStep;
    campaign.load("assets/levels/levels.txt");

    title.setFont(app.titleFont);
    title.setCharacterSize(64);
    title.setFillColor(Theme::gold());
    title.setString("VAULT HEIST");

    subtitle.setFont(app.uiFont);
    subtitle.setCharacterSize(Theme::headingSize());
    subtitle.setFillColor(Theme::dimText());
    subtitle.setString("The Living Vault");

    progressText.setFont(app.uiFont);
    progressText.setCharacterSize(Theme::textSize());

    footer.setFont(app.uiFont);
    footer.setCharacterSize(Theme::smallSize());
    footer.setFillColor(Theme::skipped());
    footer.setString("Click to select   |   Esc goes back   |   F12 saves a screenshot");

    layout();
    ready = true;
}

void MainMenuScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float centerX = (float)size.x / 2.f;
    float buttonW = 300.f;
    float buttonH = 54.f;
    float x = centerX - buttonW / 2.f;
    float y = 300.f;

    string labels[BUTTON_COUNT] = { "Campaign", "Endless Mode", "Sandbox",
                                    "Leaderboard", "Settings", "Quit" };
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i].init(&app.uiFont, labels[i], x, y + (float)i * (buttonH + 12.f),
                        buttonW, buttonH);
    }
    buttons[0].setAccent(true);
}

void MainMenuScreen::openCampaign()
{
    app.push(new CampaignMapScreen(app));
}

void MainMenuScreen::handleEvent(const sf::Event& e)
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
        app.quit();
        return;
    }

    if (buttons[0].handleEvent(e))
    {
        openCampaign();
        return;
    }
    if (buttons[1].handleEvent(e))
    {
        app.push(new EndlessScreen(app));
        return;
    }
    if (buttons[2].handleEvent(e))
    {
        app.push(new SandboxBScreen(app));
        return;
    }
    if (buttons[3].handleEvent(e))
    {
        app.push(new LeaderboardScreen(app));
        return;
    }
    if (buttons[4].handleEvent(e))
    {
        app.push(new SettingsScreen(app));
        return;
    }
    if (buttons[5].handleEvent(e))
    {
        app.quit();
    }
}

void MainMenuScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    glowClock += dt;
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i].update(dt, mouseX, mouseY);
    }
}

void MainMenuScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    sf::Vector2u size = window.getSize();
    float centerX = (float)size.x / 2.f;

    // the title breathes slightly, so the front door does not look frozen
    float pulse = 0.5f + 0.5f * sinf(glowClock * 1.6f);
    title.setFillColor(Theme::mix(Theme::gold(), Theme::text(), pulse * 0.35f));
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(centerX - tb.width / 2.f - tb.left, 120.f);
    window.draw(title);

    sf::FloatRect sb = subtitle.getLocalBounds();
    subtitle.setPosition(centerX - sb.width / 2.f - sb.left, 206.f);
    window.draw(subtitle);

    int earned = app.save.totalStars();
    int possible = campaign.count() * 3;
    progressText.setString(TextUtil::intToText(earned) + " / "
                           + TextUtil::intToText(possible) + " stars earned   -   "
                           + TextUtil::intToText(campaign.count()) + " levels across 4 acts");
    progressText.setFillColor(earned > 0 ? Theme::gold() : Theme::dimText());
    sf::FloatRect pb = progressText.getLocalBounds();
    progressText.setPosition(centerX - pb.width / 2.f - pb.left, 248.f);
    window.draw(progressText);

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i].draw(window);
    }

    sf::FloatRect fb = footer.getLocalBounds();
    footer.setPosition(centerX - fb.width / 2.f - fb.left, (float)size.y - 44.f);
    window.draw(footer);
}

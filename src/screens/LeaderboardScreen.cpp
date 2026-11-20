#include "screens/LeaderboardScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"

LeaderboardScreen::LeaderboardScreen(App& a) : app(a)
{
    mouseX = 0.f;
    mouseY = 0.f;
    ready = false;
}

void LeaderboardScreen::onEnter()
{
    campaign.load("assets/levels/levels.txt");
    title.setFont(app.titleFont);
    title.setCharacterSize(38);
    title.setFillColor(Theme::gold());
    title.setString("LEADERBOARD");
    heading.setFont(app.titleFont);
    heading.setCharacterSize(Theme::headingSize());
    line.setFont(app.monoFont);
    line.setCharacterSize(Theme::monoSize());
    layout();
    ready = true;
}

void LeaderboardScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    float panelW = 440.f;
    float panelH = 520.f;
    float y = 140.f;
    float gap = 32.f;
    float left = (w - panelW * 2.f - gap) / 2.f;

    runsPanel.setRect(left, y, panelW, panelH, Theme::panelRadius());
    runsPanel.setFillColor(Theme::panel());
    runsPanel.setOutline(Theme::panelBorder(), 1.f);
    campaignPanel.setRect(left + panelW + gap, y, panelW, panelH, Theme::panelRadius());
    campaignPanel.setFillColor(Theme::panel());
    campaignPanel.setOutline(Theme::panelBorder(), 1.f);

    backButton.init(&app.uiFont, "Back", left, h - 90.f, 200.f, 48.f);
}

void LeaderboardScreen::handleEvent(const sf::Event& e)
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
    }
}

void LeaderboardScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    backButton.update(dt, mouseX, mouseY);
}

void LeaderboardScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    sf::Vector2u size = window.getSize();
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((float)size.x / 2.f - tb.width / 2.f - tb.left, 56.f);
    window.draw(title);

    // ---- endless runs ----
    runsPanel.draw(window);
    heading.setString("Endless Runs");
    heading.setFillColor(Theme::gold());
    heading.setPosition(runsPanel.left() + 20.f, runsPanel.top() + 16.f);
    window.draw(heading);

    float y = runsPanel.top() + 62.f;
    if (app.save.scoreCount() == 0)
    {
        line.setFillColor(Theme::dimText());
        line.setString("No runs yet. Endless Mode writes here.");
        line.setPosition(runsPanel.left() + 20.f, y);
        window.draw(line);
    }
    for (int i = 0; i < app.save.scoreCount(); i++)
    {
        string rank = TextUtil::intToText(i + 1);
        if (rank.length() < 2)
        {
            rank = " " + rank;
        }
        string name = app.save.scoreName(i);
        while (name.length() < 18)
        {
            name = name + " ";
        }
        line.setString(rank + "  " + name + TextUtil::intToText(app.save.scoreValue(i)));
        line.setFillColor(i == 0 ? Theme::gold() : Theme::text());
        line.setPosition(runsPanel.left() + 20.f, y);
        window.draw(line);
        y += 26.f;
    }

    // ---- campaign record ----
    campaignPanel.draw(window);
    heading.setString("Campaign Record");
    heading.setPosition(campaignPanel.left() + 20.f, campaignPanel.top() + 16.f);
    window.draw(heading);

    y = campaignPanel.top() + 62.f;
    int totalStars = app.save.totalStars();
    line.setFillColor(Theme::gold());
    line.setString("Stars " + TextUtil::intToText(totalStars) + " / "
                   + TextUtil::intToText(campaign.count() * 3));
    line.setPosition(campaignPanel.left() + 20.f, y);
    window.draw(line);
    y += 34.f;

    for (int i = 0; i < campaign.count(); i++)
    {
        const LevelDef& def = campaign.get(i);
        int stars = app.save.starsFor(def.id);
        string starBar = "";
        for (int s = 0; s < 3; s++)
        {
            starBar = starBar + (s < stars ? "*" : "-");
        }
        string best = stars > 0 ? TextUtil::intToText(app.save.bestScoreFor(def.id)) : "-";
        line.setString(def.id + "  " + starBar + "  " + best);
        line.setFillColor(stars > 0 ? Theme::text() : Theme::skipped());
        line.setPosition(campaignPanel.left() + 20.f, y);
        window.draw(line);
        y += 21.f;
    }

    backButton.draw(window);
}

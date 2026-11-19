#include "screens/ResultScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "game/LevelFactory.h"
#include "core/TextUtil.h"

ResultScreen::ResultScreen(App& a, const string& id, const LevelOutcome& o)
    : app(a), levelId(id), outcome(o)
{
    levelIndex = -1;
    stars = ScoreSystem::computeStars(o);
    score = ScoreSystem::computeScore(o);
    hasNext = false;
    mouseX = 0.f;
    mouseY = 0.f;
    burstDone = false;
}

void ResultScreen::onEnter()
{
    campaign.load("assets/levels/levels.txt");
    levelIndex = campaign.indexOfId(levelId);
    hasNext = outcome.completed && levelIndex >= 0 && levelIndex + 1 < campaign.count();

    title.setFont(app.titleFont);
    title.setCharacterSize(40);
    subtitle.setFont(app.uiFont);
    subtitle.setCharacterSize(Theme::headingSize());
    body.setFont(app.uiFont);
    body.setCharacterSize(Theme::textSize());
    starText.setFont(app.titleFont);
    starText.setCharacterSize(56);
    layout();

    if (outcome.completed && !burstDone)
    {
        sf::Vector2u size = app.window.getSize();
        particles.burst((float)size.x / 2.f, 300.f, 200, stars * 97 + 11);
        burstDone = true;
    }
}

void ResultScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    float panelW = 620.f;
    float panelH = 470.f;
    float panelX = (w - panelW) / 2.f;
    float panelY = (h - panelH) / 2.f;
    panel.setRect(panelX, panelY, panelW, panelH, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);

    float buttonY = panelY + panelH - 76.f;
    float buttonW = 170.f;
    retryButton.init(&app.uiFont, "Retry", panelX + 32.f, buttonY, buttonW, 48.f);
    nextButton.init(&app.uiFont, hasNext ? "Next Level" : "Campaign Map",
                    panelX + panelW - buttonW - 32.f, buttonY, buttonW, 48.f);
    nextButton.setAccent(true);
    mapButton.init(&app.uiFont, "Campaign Map", panelX + (panelW - buttonW) / 2.f,
                   buttonY, buttonW, 48.f);
    mapButton.setEnabled(hasNext);
}

string ResultScreen::buildBody()
{
    string text = "Alarm steps      " + TextUtil::intToText(outcome.alarmSteps)
                + " / " + TextUtil::intToText(outcome.capacity) + "\n";
    text = text + "Optimal steps    " + TextUtil::intToText(outcome.optimalSteps) + "\n";
    text = text + "Penalties        " + TextUtil::intToText(outcome.penaltyCount);
    if (outcome.penaltyPoints > 0)
    {
        text = text + "   (-" + TextUtil::intToText(outcome.penaltyPoints) + " points)";
    }
    text = text + "\n";
    if (outcome.bonuses > 0)
    {
        text = text + "Bonuses          +" + TextUtil::intToText(outcome.bonuses) + "\n";
    }
    text = text + "Hint used        " + (outcome.hintUsed ? "yes (star cap 2)" : "no") + "\n\n";
    text = text + "Score            " + TextUtil::intToText(score);

    int best = app.save.bestScoreFor(levelId);
    if (best > 0)
    {
        text = text + "\nBest so far      " + TextUtil::intToText(best)
             + "   (" + TextUtil::intToText(app.save.starsFor(levelId)) + " stars)";
    }
    return text;
}

void ResultScreen::openLevel(int index)
{
    if (index < 0 || index >= campaign.count())
    {
        app.requestPop();
        return;
    }
    Screen* screen = LevelFactory::create(app, campaign.get(index));
    if (screen != nullptr)
    {
        app.requestReplaceTop(screen);
    }
    else
    {
        app.requestPop();
    }
}

void ResultScreen::handleEvent(const sf::Event& e)
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
    if (retryButton.handleEvent(e))
    {
        openLevel(levelIndex);
        return;
    }
    if (nextButton.handleEvent(e))
    {
        if (hasNext)
        {
            openLevel(levelIndex + 1);
        }
        else
        {
            app.requestPop();
        }
        return;
    }
    if (hasNext && mapButton.handleEvent(e))
    {
        app.requestPop();
    }
}

void ResultScreen::update(float dt)
{
    layout();
    particles.update(dt);
    retryButton.update(dt, mouseX, mouseY);
    nextButton.update(dt, mouseX, mouseY);
    if (hasNext)
    {
        mapButton.update(dt, mouseX, mouseY);
    }
}

void ResultScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    panel.drawWithShadow(window);

    float panelX = panel.left();
    float panelW = panel.width();
    float centerX = panelX + panelW / 2.f;
    float y = panel.top() + 28.f;

    title.setString(outcome.completed ? "VAULT CRACKED" : "CAUGHT");
    title.setFillColor(outcome.completed ? Theme::gold() : Theme::violation());
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(centerX - tb.width / 2.f - tb.left, y);
    window.draw(title);
    y += 58.f;

    subtitle.setString("Level " + levelId);
    subtitle.setFillColor(Theme::dimText());
    sf::FloatRect sb = subtitle.getLocalBounds();
    subtitle.setPosition(centerX - sb.width / 2.f - sb.left, y);
    window.draw(subtitle);
    y += 44.f;

    // stars, with the earned ones gold and the missed ones hollow, so the
    // count is readable without relying on the colour
    string filled = "";
    for (int i = 0; i < 3; i++)
    {
        if (i > 0)
        {
            filled = filled + " ";
        }
        filled = filled + (i < stars ? "*" : "-");
    }
    starText.setString(filled);
    starText.setFillColor(stars > 0 ? Theme::gold() : Theme::skipped());
    sf::FloatRect stb = starText.getLocalBounds();
    starText.setPosition(centerX - stb.width / 2.f - stb.left, y);
    window.draw(starText);
    y += 84.f;

    body.setString(buildBody());
    body.setFillColor(Theme::text());
    body.setPosition(panelX + 48.f, y);
    window.draw(body);

    retryButton.draw(window);
    nextButton.draw(window);
    if (hasNext)
    {
        mapButton.draw(window);
    }
    particles.draw(window);
}

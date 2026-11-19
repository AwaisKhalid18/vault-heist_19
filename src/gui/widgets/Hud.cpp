#include "gui/widgets/Hud.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"
#include <cmath>

Hud::Hud()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    alarmSteps = 0;
    alarmCapacity = 10;
    scoreValue = 0;
    stepsValue = 0;
    optimalValue = 0;
    shownFill = 0.f;
    pulseClock = 0.f;
}

void Hud::init(sf::Font* headingFont, sf::Font* textFont)
{
    title.setFont(*headingFont);
    title.setCharacterSize(Theme::headingSize());
    title.setFillColor(Theme::gold());
    title.setString("OBJECTIVE");
    objective.setFont(*textFont);
    objective.setCharacterSize(Theme::smallSize());
    objective.setFillColor(Theme::text());
    alarmLabel.setFont(*textFont);
    alarmLabel.setCharacterSize(Theme::smallSize());
    alarmLabel.setFillColor(Theme::text());
    scoreText.setFont(*textFont);
    scoreText.setCharacterSize(Theme::smallSize());
    scoreText.setFillColor(Theme::dimText());
}

void Hud::setRect(float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    panel.setRect(x, y, width, height, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);
}

void Hud::setObjective(const string& text)
{
    // simple word wrap to the panel width
    string wrapped = "";
    string word = "";
    string line = "";
    int maxChars = (int)((sizeW - 36.f) / 7.f);
    for (int i = 0; i <= (int)text.length(); i++)
    {
        char c = i < (int)text.length() ? text[i] : ' ';
        if (c == ' ')
        {
            if ((int)(line.length() + word.length()) + 1 > maxChars && line.length() > 0)
            {
                wrapped = wrapped + line + "\n";
                line = "";
            }
            if (line.length() > 0)
            {
                line = line + " ";
            }
            line = line + word;
            word = "";
        }
        else
        {
            word = word + c;
        }
    }
    wrapped = wrapped + line;
    objective.setString(wrapped);
}

void Hud::setAlarm(int steps, int capacity)
{
    alarmSteps = steps;
    alarmCapacity = capacity > 0 ? capacity : 1;
}

void Hud::setScoreInfo(int score, int steps, int optimal)
{
    scoreValue = score;
    stepsValue = steps;
    optimalValue = optimal;
}

void Hud::update(float dt)
{
    pulseClock += dt;
    float target = (float)alarmSteps / (float)alarmCapacity;
    if (target > 1.f)
    {
        target = 1.f;
    }
    shownFill += (target - shownFill) * (6.f * dt < 1.f ? 6.f * dt : 1.f);
}

void Hud::draw(sf::RenderWindow& window)
{
    panel.drawWithShadow(window);

    float x = posX + 18.f;
    float y = posY + 12.f;
    title.setPosition(x, y);
    window.draw(title);
    y += 34.f;
    objective.setPosition(x, y);
    window.draw(objective);
    sf::FloatRect ob = objective.getLocalBounds();
    y += ob.height + 18.f;

    // alarm meter
    alarmLabel.setString("ALARM  " + TextUtil::intToText(alarmSteps) + " / "
                         + TextUtil::intToText(alarmCapacity));
    alarmLabel.setPosition(x, y);
    window.draw(alarmLabel);
    y += 22.f;

    float barW = sizeW - 36.f;
    float barH = 12.f;
    RoundedRect track;
    track.setRect(x, y, barW, barH, 6.f);
    track.setFillColor(sf::Color(10, 14, 22));
    track.setOutline(Theme::panelBorder(), 1.f);
    track.draw(window);

    float fillW = barW * shownFill;
    if (fillW > 3.f)
    {
        sf::Color fillColor = Theme::mix(Theme::alarmLow(), Theme::alarmHigh(), shownFill);
        if (shownFill > 0.8f)
        {
            // pulse when nearly full
            float pulse = 0.5f + 0.5f * sinf(pulseClock * 8.f);
            fillColor = Theme::mix(fillColor, sf::Color::White, pulse * 0.25f);
        }
        RoundedRect fill;
        fill.setRect(x, y, fillW, barH, 6.f);
        fill.setFillColor(fillColor);
        fill.draw(window);
    }
    y += barH + 14.f;

    scoreText.setString("Score " + TextUtil::intToText(scoreValue)
                        + "    Steps " + TextUtil::intToText(stepsValue)
                        + "    Optimal " + TextUtil::intToText(optimalValue));
    scoreText.setPosition(x, y);
    window.draw(scoreText);
}

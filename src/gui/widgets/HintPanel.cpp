#include "gui/widgets/HintPanel.h"
#include "gui/Theme.h"

HintPanel::HintPanel()
{
    slide = 0.f;
    visible = false;
    windowWidth = 1600.f;
    text = "";
}

void HintPanel::init(sf::Font* headingFont, sf::Font* textFont)
{
    title.setFont(*headingFont);
    title.setCharacterSize(Theme::headingSize());
    title.setFillColor(Theme::warning());
    title.setString("HINT");
    body.setFont(*textFont);
    body.setCharacterSize(Theme::smallSize());
    body.setFillColor(Theme::text());
    body.setLineSpacing(1.2f);
}

void HintPanel::show(const string& hintText)
{
    // wrap to the card width
    string wrapped = "";
    string word = "";
    string line = "";
    for (int i = 0; i <= (int)hintText.length(); i++)
    {
        char c = i < (int)hintText.length() ? hintText[i] : ' ';
        if (c == ' ')
        {
            if ((int)(line.length() + word.length()) + 1 > 42 && line.length() > 0)
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
    text = wrapped;
    body.setString(wrapped);
    visible = true;
}

void HintPanel::hide()
{
    visible = false;
}

bool HintPanel::isVisible()
{
    return visible;
}

void HintPanel::update(float dt, float windowW)
{
    windowWidth = windowW;
    float target = visible ? 1.f : 0.f;
    float step = dt * 5.f;
    if (slide < target)
    {
        slide = slide + step < target ? slide + step : target;
    }
    else
    {
        slide = slide - step > target ? slide - step : target;
    }
}

void HintPanel::draw(sf::RenderWindow& window)
{
    if (slide <= 0.01f)
    {
        return;
    }
    sf::FloatRect bodyBounds = body.getLocalBounds();
    float w = 360.f;
    float h = 70.f + bodyBounds.height;
    float eased = Theme::ease(slide);
    float x = windowWidth - w * eased - 10.f * eased;
    float y = 90.f;

    panel.setRect(x, y, w, h, Theme::panelRadius());
    panel.setFillColor(sf::Color(26, 30, 24, 245));
    panel.setOutline(Theme::warning(), 1.f);
    panel.drawWithShadow(window);
    title.setPosition(x + 20.f, y + 10.f);
    window.draw(title);
    body.setPosition(x + 20.f, y + 44.f);
    window.draw(body);
}

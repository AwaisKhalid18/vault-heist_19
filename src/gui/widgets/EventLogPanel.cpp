#include "gui/widgets/EventLogPanel.h"
#include "gui/Theme.h"

EventLogPanel::EventLogPanel()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    scrollOffset = 0;
    userScrolled = false;
}

void EventLogPanel::init(sf::Font* headingFont, sf::Font* monoFont)
{
    title.setFont(*headingFont);
    title.setCharacterSize(Theme::headingSize());
    title.setFillColor(Theme::gold());
    title.setString("EVENT LOG");
    line.setFont(*monoFont);
    line.setCharacterSize(Theme::monoSize());
}

void EventLogPanel::setRect(float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    panel.setRect(x, y, width, height, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);
}

bool EventLogPanel::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseWheelScrolled)
    {
        float mx = (float)e.mouseWheelScroll.x;
        float my = (float)e.mouseWheelScroll.y;
        if (panel.contains(mx, my))
        {
            scrollOffset -= (int)e.mouseWheelScroll.delta * 3;
            if (scrollOffset < 0)
            {
                scrollOffset = 0;
            }
            userScrolled = true;
            return true;
        }
    }
    return false;
}

void EventLogPanel::draw(sf::RenderWindow& window, EventLog* log, int currentIndex)
{
    panel.drawWithShadow(window);
    title.setPosition(posX + 18.f, posY + 10.f);
    window.draw(title);

    if (log == nullptr)
    {
        return;
    }
    float lineHeight = 20.f;
    float top = posY + 46.f;
    int visible = (int)((sizeH - 56.f) / lineHeight);
    int total = log->size();

    // follow the animation unless the user scrolled by hand
    if (!userScrolled || currentIndex < scrollOffset || currentIndex >= scrollOffset + visible)
    {
        scrollOffset = currentIndex - visible + 2;
        if (scrollOffset < 0)
        {
            scrollOffset = 0;
        }
        userScrolled = false;
    }
    if (scrollOffset > total - 1 && total > 0)
    {
        scrollOffset = total - 1;
    }

    for (int i = 0; i < visible; i++)
    {
        int index = scrollOffset + i;
        if (index >= total)
        {
            break;
        }
        const Event& e = log->get(index);
        string text = e.text;
        if ((int)text.length() > 40)
        {
            text = text.substr(0, 39) + "~";
        }
        line.setString(text);
        line.setPosition(posX + 18.f, top + (float)i * lineHeight);
        if (index == currentIndex)
        {
            // highlight bar behind the active line
            sf::RectangleShape bar(sf::Vector2f(sizeW - 24.f, lineHeight));
            bar.setPosition(posX + 12.f, top + (float)i * lineHeight - 2.f);
            bar.setFillColor(Theme::withAlpha(Theme::currentVisit(), 40));
            window.draw(bar);
            line.setFillColor(Theme::text());
        }
        else if (index < currentIndex)
        {
            line.setFillColor(Theme::dimText());
        }
        else
        {
            line.setFillColor(Theme::withAlpha(Theme::dimText(), 120));
        }
        window.draw(line);
    }
}

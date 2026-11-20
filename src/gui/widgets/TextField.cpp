#include "gui/widgets/TextField.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"

TextField::TextField()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    focused = false;
    caretBlink = 0.f;
    buffer = "";
    placeholderText = "";
}

void TextField::init(sf::Font* font, const string& placeholder, float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    placeholderText = placeholder;
    rect.setRect(x, y, width, height, 8.f);
    text.setFont(*font);
    text.setCharacterSize(Theme::textSize());
    text.setFillColor(Theme::text());
    hint.setFont(*font);
    hint.setCharacterSize(Theme::textSize());
    hint.setFillColor(Theme::dimText());
    hint.setString(placeholder);
}

bool TextField::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        focused = rect.contains((float)e.mouseButton.x, (float)e.mouseButton.y);
    }
    if (!focused)
    {
        return false;
    }
    if (e.type == sf::Event::TextEntered)
    {
        unsigned int code = e.text.unicode;
        if (code == 8)   // backspace
        {
            if (buffer.length() > 0)
            {
                buffer = buffer.substr(0, buffer.length() - 1);
            }
        }
        else if (code == 13)   // enter
        {
            return true;
        }
        else if (code >= '0' && code <= '9' && buffer.length() < 9)
        {
            buffer = buffer + (char)code;
        }
        else if (code == '-' && buffer.length() == 0)
        {
            buffer = "-";
        }
    }
    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter)
    {
        return true;
    }
    return false;
}

void TextField::update(float dt)
{
    caretBlink += dt;
    if (caretBlink > 1.f)
    {
        caretBlink -= 1.f;
    }
}

void TextField::draw(sf::RenderWindow& window)
{
    rect.setFillColor(sf::Color(10, 14, 22));
    rect.setOutline(focused ? Theme::currentVisit() : Theme::panelBorder(), focused ? 2.f : 1.f);
    rect.draw(window);

    float textY = posY + (sizeH - (float)Theme::textSize()) / 2.f - 3.f;
    if (buffer.length() == 0 && !focused)
    {
        hint.setPosition(posX + 10.f, textY);
        window.draw(hint);
    }
    else
    {
        string shown = buffer;
        if (focused && caretBlink < 0.5f)
        {
            shown = shown + "|";
        }
        text.setString(shown);
        text.setPosition(posX + 10.f, textY);
        window.draw(text);
    }
}

bool TextField::getInt(int& out)
{
    bool ok = false;
    int value = TextUtil::textToInt(buffer, ok);
    if (ok)
    {
        out = value;
    }
    return ok;
}

void TextField::clearText()
{
    buffer = "";
}

void TextField::setFocused(bool on)
{
    focused = on;
}

bool TextField::isFocused()
{
    return focused;
}

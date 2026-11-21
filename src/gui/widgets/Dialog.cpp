#include "gui/widgets/Dialog.h"
#include "gui/Theme.h"

Dialog::Dialog()
{
    buttonCount = 0;
    openFlag = false;
    fade = 0.f;
    titleFontPtr = nullptr;
    textFontPtr = nullptr;
}

void Dialog::init(sf::Font* titleFont, sf::Font* textFont)
{
    titleFontPtr = titleFont;
    textFontPtr = textFont;
    title.setFont(*titleFont);
    title.setCharacterSize(26);
    title.setFillColor(Theme::gold());
    body.setFont(*textFont);
    body.setCharacterSize(Theme::textSize());
    body.setFillColor(Theme::text());
    body.setLineSpacing(1.25f);
}

string Dialog::wrap(const string& text, int maxChars)
{
    string wrapped = "";
    string word = "";
    string line = "";
    for (int i = 0; i <= (int)text.length(); i++)
    {
        char c = i < (int)text.length() ? text[i] : ' ';
        if (c == '\n')
        {
            // keep explicit line breaks
            if (line.length() > 0)
            {
                line = line + (line.length() > 0 && word.length() > 0 ? " " : "");
            }
            line = line + word;
            wrapped = wrapped + line + "\n";
            line = "";
            word = "";
        }
        else if (c == ' ')
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
    return wrapped;
}

void Dialog::open(const string& titleText, const string& bodyText,
                  const string buttonLabels[], int count)
{
    pendingTitle = titleText;
    pendingBody = bodyText;
    if (count > MAX_DIALOG_BUTTONS)
    {
        count = MAX_DIALOG_BUTTONS;
    }
    buttonCount = count;
    for (int i = 0; i < count; i++)
    {
        pendingLabels[i] = buttonLabels[i];
    }
    openFlag = true;
    fade = 0.f;
}

void Dialog::open2(const string& titleText, const string& bodyText,
                   const string& button1, const string& button2)
{
    string labels[2];
    labels[0] = button1;
    labels[1] = button2;
    open(titleText, bodyText, labels, button2.length() > 0 ? 2 : 1);
}

void Dialog::close()
{
    openFlag = false;
}

bool Dialog::isOpen()
{
    return openFlag;
}

void Dialog::layout(float windowW, float windowH)
{
    // the panel has to be wide enough to hold its row of buttons
    float buttonW = 150.f;
    float buttonGap = 14.f;
    float buttonsW = (float)buttonCount * buttonW + (float)(buttonCount - 1) * buttonGap;
    float w = 640.f;
    if (buttonsW + 64.f > w)
    {
        w = buttonsW + 64.f;
    }
    title.setString(pendingTitle);
    body.setString(wrap(pendingBody, 64));
    sf::FloatRect bodyBounds = body.getLocalBounds();
    float h = 120.f + bodyBounds.height + 70.f;
    if (h < 240.f)
    {
        h = 240.f;
    }
    // The panel sits low rather than dead centre, so the strained part of the
    // tree stays visible while the question is on screen. It is also kept
    // clear of the toolbar along the bottom.
    float x = (windowW - w) / 2.f;
    float free = windowH - h;
    if (free < 0.f)
    {
        free = 0.f;
    }
    float y = free * 0.74f;
    float lowest = windowH - h - 86.f;
    if (y > lowest)
    {
        y = lowest;
    }
    if (y < 16.f)
    {
        y = 16.f;
    }
    panel.setRect(x, y, w, h, Theme::panelRadius());
    panel.setFillColor(sf::Color(18, 24, 37, 224));
    panel.setOutline(Theme::gold(), 1.f);
    title.setPosition(x + 32.f, y + 24.f);
    body.setPosition(x + 32.f, y + 86.f);

    float bx = x + (w - buttonsW) / 2.f;
    float by = y + h - 58.f;
    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].init(textFontPtr, pendingLabels[i], bx, by, buttonW, 40.f);
        if (i == 0)
        {
            buttons[i].setAccent(true);
        }
        bx += buttonW + buttonGap;
    }
}

int Dialog::handleEvent(const sf::Event& e)
{
    if (!openFlag)
    {
        return -1;
    }
    for (int i = 0; i < buttonCount; i++)
    {
        if (buttons[i].handleEvent(e))
        {
            return i;
        }
    }
    return -1;
}

void Dialog::update(float dt, float mouseX, float mouseY)
{
    if (!openFlag)
    {
        return;
    }
    fade += dt * 5.f;
    if (fade > 1.f)
    {
        fade = 1.f;
    }
    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].update(dt, mouseX, mouseY);
    }
}

void Dialog::draw(sf::RenderWindow& window)
{
    if (!openFlag)
    {
        return;
    }
    sf::Vector2u size = window.getSize();
    layout((float)size.x, (float)size.y);

    // A light dim only: enough to lift the panel off the background without
    // hiding the tree the question is asking about.
    sf::RectangleShape dimmer(sf::Vector2f((float)size.x, (float)size.y));
    dimmer.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(64.f * fade)));
    window.draw(dimmer);

    panel.drawWithShadow(window);
    window.draw(title);
    window.draw(body);
    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].draw(window);
    }
}

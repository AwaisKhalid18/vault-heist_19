#include "gui/widgets/OperationPanel.h"
#include "gui/Theme.h"

OperationPanel::OperationPanel()
{
    buttonCount = 0;
    fieldCount = 0;
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    textFont = nullptr;
    placeholderA = "key";
    placeholderB = "hi";
}

void OperationPanel::init(sf::Font* headingFont, sf::Font* font, const string& titleText)
{
    textFont = font;
    title.setFont(*headingFont);
    title.setCharacterSize(Theme::headingSize());
    title.setFillColor(Theme::gold());
    title.setString(titleText);
}

void OperationPanel::setRect(float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    panel.setRect(x, y, width, height, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);
    relayout();
}

void OperationPanel::clearButtons()
{
    buttonCount = 0;
}

int OperationPanel::addButton(const string& label)
{
    if (buttonCount >= MAX_BUTTONS)
    {
        return -1;
    }
    labels[buttonCount] = label;
    buttonCount++;
    return buttonCount - 1;
}

void OperationPanel::setFieldCount(int count)
{
    fieldCount = count;
}

void OperationPanel::setFieldPlaceholders(const string& a, const string& b)
{
    placeholderA = a;
    placeholderB = b;
}

void OperationPanel::relayout()
{
    if (textFont == nullptr || sizeW <= 0.f)
    {
        return;
    }
    float x = posX + 14.f;
    float y = posY + 46.f;
    float innerW = sizeW - 28.f;

    if (fieldCount >= 1)
    {
        float fieldW = fieldCount == 2 ? innerW / 2.f - 5.f : innerW;
        fieldA.init(textFont, placeholderA, x, y, fieldW, 34.f);
        if (fieldCount == 2)
        {
            fieldB.init(textFont, placeholderB, x + fieldW + 10.f, y, fieldW, 34.f);
        }
        y += 44.f;
    }

    // two columns of buttons
    float buttonW = innerW / 2.f - 5.f;
    float buttonH = 34.f;
    for (int i = 0; i < buttonCount; i++)
    {
        float bx = x + (float)(i % 2) * (buttonW + 10.f);
        float by = y + (float)(i / 2) * (buttonH + 8.f);
        buttons[i].init(textFont, labels[i], bx, by, buttonW, buttonH);
    }
}

int OperationPanel::handleEvent(const sf::Event& e)
{
    if (fieldCount >= 1)
    {
        fieldA.handleEvent(e);
    }
    if (fieldCount >= 2)
    {
        fieldB.handleEvent(e);
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

void OperationPanel::update(float dt, float mouseX, float mouseY)
{
    if (fieldCount >= 1)
    {
        fieldA.update(dt);
    }
    if (fieldCount >= 2)
    {
        fieldB.update(dt);
    }
    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].update(dt, mouseX, mouseY);
    }
}

void OperationPanel::draw(sf::RenderWindow& window)
{
    panel.drawWithShadow(window);
    title.setPosition(posX + 18.f, posY + 10.f);
    window.draw(title);
    if (fieldCount >= 1)
    {
        fieldA.draw(window);
    }
    if (fieldCount >= 2)
    {
        fieldB.draw(window);
    }
    for (int i = 0; i < buttonCount; i++)
    {
        buttons[i].draw(window);
    }
}

bool OperationPanel::getFieldInt(int field, int& out)
{
    if (field == 0 && fieldCount >= 1)
    {
        return fieldA.getInt(out);
    }
    if (field == 1 && fieldCount >= 2)
    {
        return fieldB.getInt(out);
    }
    return false;
}

void OperationPanel::clearFields()
{
    fieldA.clearText();
    fieldB.clearText();
}

void OperationPanel::setButtonEnabled(int index, bool on)
{
    if (index >= 0 && index < buttonCount)
    {
        buttons[index].setEnabled(on);
    }
}

void OperationPanel::setButtonToggled(int index, bool on)
{
    if (index >= 0 && index < buttonCount)
    {
        buttons[index].setToggled(on);
    }
}

bool OperationPanel::anyFieldFocused()
{
    if (fieldCount >= 1 && fieldA.isFocused())
    {
        return true;
    }
    if (fieldCount >= 2 && fieldB.isFocused())
    {
        return true;
    }
    return false;
}

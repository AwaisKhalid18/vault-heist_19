#include "screens/SettingsScreen.h"
#include "gui/App.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"

SettingsScreen::SettingsScreen(App& a) : app(a)
{
    mouseX = 0.f;
    mouseY = 0.f;
    confirmingReset = false;
    ready = false;
}

void SettingsScreen::onEnter()
{
    title.setFont(app.titleFont);
    title.setCharacterSize(38);
    title.setFillColor(Theme::gold());
    title.setString("SETTINGS");
    label.setFont(app.uiFont);
    label.setCharacterSize(Theme::textSize());
    layout();

    // the slider runs 0.25x to 4x, so the stored speed maps onto 0..1
    float value = (app.save.animationSpeed - 0.25f) / 3.75f;
    speedSlider.setValue(value);
    refreshLabels();
    ready = true;
}

void SettingsScreen::layout()
{
    sf::Vector2u size = app.window.getSize();
    float w = (float)size.x;
    float h = (float)size.y;
    float panelW = 660.f;
    float panelH = 440.f;
    float x = (w - panelW) / 2.f;
    float y = (h - panelH) / 2.f;
    panel.setRect(x, y, panelW, panelH, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);

    speedSlider.init(x + 300.f, y + 86.f, 280.f);
    float buttonX = x + 300.f;
    float buttonW = 280.f;
    validateButton.init(&app.uiFont, "", buttonX, y + 126.f, buttonW, 44.f);
    tooltipButton.init(&app.uiFont, "", buttonX, y + 182.f, buttonW, 44.f);
    unlockButton.init(&app.uiFont, "", buttonX, y + 238.f, buttonW, 44.f);
    resetButton.init(&app.uiFont, "", buttonX, y + 294.f, buttonW, 44.f);
    backButton.init(&app.uiFont, "Back", x + 32.f, y + panelH - 72.f, 180.f, 48.f);
}

float SettingsScreen::sliderToSpeed()
{
    return 0.25f + speedSlider.getValue() * 3.75f;
}

void SettingsScreen::refreshLabels()
{
    validateButton.setLabel(app.save.validateAfterEachStep ? "On" : "Off");
    validateButton.setToggled(app.save.validateAfterEachStep);
    tooltipButton.setLabel(app.save.showTooltips ? "On" : "Off");
    tooltipButton.setToggled(app.save.showTooltips);
    unlockButton.setLabel(app.save.unlockAllLevels ? "Every level open" : "Unlock as you go");
    unlockButton.setToggled(app.save.unlockAllLevels);
    resetButton.setLabel(confirmingReset ? "Press again to wipe" : "Erase all progress");
    resetButton.setAccent(confirmingReset);
}

void SettingsScreen::storeAndSave()
{
    app.save.animationSpeed = sliderToSpeed();
    app.debugValidate = app.save.validateAfterEachStep;
    app.save.save(app.savePath);
}

void SettingsScreen::handleEvent(const sf::Event& e)
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
        storeAndSave();
        app.requestPop();
        return;
    }
    if (speedSlider.handleEvent(e))
    {
        app.save.animationSpeed = sliderToSpeed();
        return;
    }
    if (validateButton.handleEvent(e))
    {
        app.save.validateAfterEachStep = !app.save.validateAfterEachStep;
        confirmingReset = false;
        refreshLabels();
        storeAndSave();
        return;
    }
    if (tooltipButton.handleEvent(e))
    {
        app.save.showTooltips = !app.save.showTooltips;
        confirmingReset = false;
        refreshLabels();
        storeAndSave();
        return;
    }
    if (unlockButton.handleEvent(e))
    {
        app.save.unlockAllLevels = !app.save.unlockAllLevels;
        confirmingReset = false;
        refreshLabels();
        storeAndSave();
        return;
    }
    if (resetButton.handleEvent(e))
    {
        // wiping the save needs two clicks, because it cannot be undone
        if (!confirmingReset)
        {
            confirmingReset = true;
        }
        else
        {
            confirmingReset = false;
            bool keepUnlock = app.save.unlockAllLevels;
            float keepSpeed = app.save.animationSpeed;
            app.save.clear();
            app.save.unlockAllLevels = keepUnlock;
            app.save.animationSpeed = keepSpeed;
            storeAndSave();
        }
        refreshLabels();
        return;
    }
    if (backButton.handleEvent(e))
    {
        storeAndSave();
        app.requestPop();
    }
}

void SettingsScreen::update(float dt)
{
    if (!ready)
    {
        return;
    }
    validateButton.update(dt, mouseX, mouseY);
    tooltipButton.update(dt, mouseX, mouseY);
    unlockButton.update(dt, mouseX, mouseY);
    resetButton.update(dt, mouseX, mouseY);
    backButton.update(dt, mouseX, mouseY);
}

void SettingsScreen::draw(sf::RenderWindow& window)
{
    app.useUiView();
    background.draw(window);
    sf::Vector2u size = window.getSize();
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((float)size.x / 2.f - tb.width / 2.f - tb.left, panel.top() - 76.f);
    window.draw(title);

    panel.drawWithShadow(window);
    float textX = panel.left() + 32.f;

    label.setFillColor(Theme::text());
    label.setString("Animation speed");
    label.setPosition(textX, panel.top() + 76.f);
    window.draw(label);
    label.setFillColor(Theme::gold());
    label.setString(TextUtil::intToText((int)(sliderToSpeed() * 100.f)) + "%");
    label.setPosition(panel.left() + panel.width() - 90.f, panel.top() + 76.f);
    window.draw(label);

    label.setFillColor(Theme::text());
    label.setString("Run the validator after every step");
    label.setPosition(textX, panel.top() + 136.f);
    window.draw(label);

    label.setString("Show room tooltips");
    label.setPosition(textX, panel.top() + 192.f);
    window.draw(label);

    label.setString("Level unlocking");
    label.setPosition(textX, panel.top() + 248.f);
    window.draw(label);

    label.setString("Campaign progress");
    label.setPosition(textX, panel.top() + 304.f);
    window.draw(label);

    speedSlider.draw(window);
    validateButton.draw(window);
    tooltipButton.draw(window);
    unlockButton.draw(window);
    resetButton.draw(window);
    backButton.draw(window);
}

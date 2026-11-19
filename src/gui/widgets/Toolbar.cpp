#include "gui/widgets/Toolbar.h"
#include "gui/Theme.h"
#include "core/TextUtil.h"

Toolbar::Toolbar()
{
    posX = 0.f;
    posY = 0.f;
    sizeW = 0.f;
    sizeH = 0.f;
    scrubbed = false;
    ready = false;
    font = nullptr;
}

void Toolbar::init(sf::Font* f)
{
    font = f;
    speedLabel.setFont(*f);
    speedLabel.setCharacterSize(Theme::smallSize());
    speedLabel.setFillColor(Theme::dimText());
    scrubLabel.setFont(*f);
    scrubLabel.setCharacterSize(Theme::smallSize());
    scrubLabel.setFillColor(Theme::dimText());
    speedSlider.setValue(0.2f);   // maps to 1x below
}

void Toolbar::setRect(float x, float y, float width, float height)
{
    posX = x;
    posY = y;
    sizeW = width;
    sizeH = height;
    panel.setRect(x, y, width, height, Theme::panelRadius());
    panel.setFillColor(Theme::panel());
    panel.setOutline(Theme::panelBorder(), 1.f);

    float bx = x + 14.f;
    float by = y + 12.f;
    float bh = height - 24.f;
    backButton.init(font, "<", bx, by, 40.f, bh);
    bx += 48.f;
    playButton.init(font, "Play", bx, by, 76.f, bh);
    bx += 84.f;
    fwdButton.init(font, ">", bx, by, 40.f, bh);
    bx += 56.f;
    undoButton.init(font, "Undo (Z)", bx, by, 92.f, bh);
    bx += 100.f;
    replayButton.init(font, "Replay", bx, by, 82.f, bh);
    bx += 90.f;
    resetButton.init(font, "Reset", bx, by, 76.f, bh);
    bx += 84.f;
    hintButton.init(font, "Hint (H)", bx, by, 88.f, bh);
    bx += 96.f;
    fitButton.init(font, "Fit (F)", bx, by, 72.f, bh);
    bx += 88.f;

    speedLabelX = bx;
    speedLabelY = y + height / 2.f - 9.f;
    speedSlider.init(bx + 56.f, y + height / 2.f, 110.f);
    float scrubX = bx + 196.f;
    float scrubW = width - (scrubX - x) - 24.f;
    if (scrubW < 60.f)
    {
        scrubW = 60.f;
    }
    scrubSlider.init(scrubX, y + height / 2.f, scrubW);
    ready = true;
}

float Toolbar::getSpeed()
{
    // slider 0..1 maps to 0.25x..4x, with 1x near the left fifth
    float v = speedSlider.getValue();
    return 0.25f + v * 3.75f;
}

float Toolbar::getScrub()
{
    return scrubSlider.getValue();
}

int Toolbar::handleEvent(const sf::Event& e)
{
    if (!ready)
    {
        return ACT_NONE;
    }
    if (playButton.handleEvent(e)) { return ACT_PLAY; }
    if (backButton.handleEvent(e)) { return ACT_STEP_BACK; }
    if (fwdButton.handleEvent(e)) { return ACT_STEP_FWD; }
    if (undoButton.handleEvent(e)) { return ACT_UNDO; }
    if (replayButton.handleEvent(e)) { return ACT_REPLAY; }
    if (resetButton.handleEvent(e)) { return ACT_RESET; }
    if (hintButton.handleEvent(e)) { return ACT_HINT; }
    if (fitButton.handleEvent(e)) { return ACT_FIT; }
    if (speedSlider.handleEvent(e)) { return ACT_SPEED; }
    if (scrubSlider.handleEvent(e))
    {
        scrubbed = true;
        return ACT_SCRUB;
    }
    return ACT_NONE;
}

void Toolbar::update(float dt, float mouseX, float mouseY, bool playing, float progress)
{
    if (!ready)
    {
        return;
    }
    playButton.setLabel(playing ? "Pause" : "Play");
    playButton.update(dt, mouseX, mouseY);
    backButton.update(dt, mouseX, mouseY);
    fwdButton.update(dt, mouseX, mouseY);
    undoButton.update(dt, mouseX, mouseY);
    replayButton.update(dt, mouseX, mouseY);
    resetButton.update(dt, mouseX, mouseY);
    hintButton.update(dt, mouseX, mouseY);
    fitButton.update(dt, mouseX, mouseY);
    if (!scrubbed)
    {
        scrubSlider.setValue(progress);   // follow the animation
    }
    scrubbed = false;
}

void Toolbar::draw(sf::RenderWindow& window)
{
    if (!ready)
    {
        return;
    }
    panel.drawWithShadow(window);
    playButton.draw(window);
    backButton.draw(window);
    fwdButton.draw(window);
    undoButton.draw(window);
    replayButton.draw(window);
    resetButton.draw(window);
    hintButton.draw(window);
    fitButton.draw(window);

    // label, then the sliders
    float speed = getSpeed();
    int whole = (int)speed;
    int frac = (int)((speed - (float)whole) * 100.f + 0.5f);
    string speedText = TextUtil::intToText(whole) + "." + (frac < 10 ? "0" : "") + TextUtil::intToText(frac) + "x";
    speedLabel.setString(speedText);
    speedLabel.setPosition(speedLabelX, speedLabelY);
    window.draw(speedLabel);
    speedSlider.draw(window);
    scrubSlider.draw(window);
}

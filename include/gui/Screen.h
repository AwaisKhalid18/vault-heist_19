#pragma once

#include <SFML/Graphics.hpp>

// Base class for every screen (menus, levels, sandbox).
// The App keeps a stack of these and always runs the top one.
class Screen
{
public:
    virtual ~Screen()
    {
    }

    virtual void onEnter()
    {
    }

    virtual void handleEvent(const sf::Event& e)
    {
    }

    virtual void update(float dt)
    {
    }

    virtual void draw(sf::RenderWindow& window)
    {
    }
};

#pragma once

#include <SFML/Audio.hpp>
#include <string>

using namespace std;

// All the sound in one place: the two interface clicks and the menu theme.
//
// The hover and click effects are generated as samples at start-up rather
// than loaded from files, so the repository needs no extra sound assets and
// the two cues always match each other.
//
// The widgets reach this through static functions. Button is created in a
// dozen screens and is copied into arrays, so threading an Audio& through
// every init() would touch far more code than the feature is worth; the one
// instance lives inside audioInstance() in the .cpp, not at file scope.
class Audio
{
public:
    // Builds the effect buffers and opens the music file. Safe to call once
    // from App::init. Returns false only if the music file is missing, and
    // the effects still work in that case.
    static bool init();

    static void playHover();
    static void playClick();

    // The theme. It plays throughout the whole game - menus, levels and
    // sandbox alike - starting at the chosen highlight and looping that
    // section for as long as the game is open. Calling startMusic when it
    // is already playing does nothing, so the run loop can call it freely.
    static void startMusic();
    static void stopMusic();
    static bool musicPlaying();

    static void setEffectsVolume(float v);   // 0..100
    static void setMusicVolume(float v);     // 0..100
};

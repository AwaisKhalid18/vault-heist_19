#include "gui/Audio.h"
#include <cmath>

static const float AU_PI = 3.14159265f;
static const unsigned int SAMPLE_RATE = 44100;

// The best part of the menu theme: start here and loop to the end.
static const float MUSIC_START_SECONDS = 485.f;   // 8:05

namespace
{
    // One instance, created on first use and destroyed at exit. Kept inside a
    // function rather than at file scope so nothing is constructed before
    // main() runs.
    struct AudioState
    {
        sf::SoundBuffer hoverBuffer;
        sf::SoundBuffer clickBuffer;
        sf::Sound hoverSound;
        sf::Sound clickSound;
        sf::Music music;
        bool musicReady = false;
        bool ready = false;
    };

    AudioState& audioInstance()
    {
        static AudioState state;
        return state;
    }

    // An exponentially decaying blend of two partials. Short, soft and dry:
    // these play on every hover, so anything resonant becomes tiring fast.
    void makeTone(sf::SoundBuffer& out, float baseHz, float overtoneHz,
                  float overtoneMix, float seconds, float decay, float amplitude)
    {
        int count = (int)(seconds * (float)SAMPLE_RATE);
        if (count < 1) { count = 1; }

        sf::Int16* samples = new sf::Int16[count];
        for (int i = 0; i < count; i++)
        {
            float t = (float)i / (float)SAMPLE_RATE;
            float env = expf(-decay * t);

            // a 3 ms fade-in removes the click that a hard start would make
            float attack = t < 0.003f ? t / 0.003f : 1.f;

            float wave = sinf(2.f * AU_PI * baseHz * t) * (1.f - overtoneMix)
                       + sinf(2.f * AU_PI * overtoneHz * t) * overtoneMix;

            float v = wave * env * attack * amplitude;
            if (v > 1.f) { v = 1.f; }
            if (v < -1.f) { v = -1.f; }
            samples[i] = (sf::Int16)(v * 32000.f);
        }
        out.loadFromSamples(samples, (sf::Uint64)count, 1, SAMPLE_RATE);
        delete[] samples;
    }
}

bool Audio::init()
{
    AudioState& a = audioInstance();
    if (a.ready)
    {
        return a.musicReady;
    }

    // hover: a light, high tick
    makeTone(a.hoverBuffer, 1180.f, 2360.f, 0.25f, 0.075f, 58.f, 0.22f);
    // click: lower and firmer, with a metallic fifth above it
    makeTone(a.clickBuffer, 440.f, 660.f, 0.38f, 0.150f, 26.f, 0.42f);

    a.hoverSound.setBuffer(a.hoverBuffer);
    a.clickSound.setBuffer(a.clickBuffer);
    a.hoverSound.setVolume(55.f);
    a.clickSound.setVolume(80.f);

    // run from the build folder or from the repository root
    a.musicReady = a.music.openFromFile("assets/sound/Light of the Seven.mp3")
                || a.music.openFromFile("../assets/sound/Light of the Seven.mp3");

    if (a.musicReady)
    {
        sf::Time total = a.music.getDuration();
        sf::Time from = sf::seconds(MUSIC_START_SECONDS);
        if (from >= total)
        {
            from = sf::Time::Zero;      // a shorter file than expected: play it all
        }
        a.music.setLoopPoints(sf::Music::TimeSpan(from, total - from));
        a.music.setLoop(true);
        a.music.setVolume(42.f);
    }

    a.ready = true;
    return a.musicReady;
}

void Audio::playHover()
{
    AudioState& a = audioInstance();
    if (!a.ready) { return; }
    a.hoverSound.play();
}

void Audio::playClick()
{
    AudioState& a = audioInstance();
    if (!a.ready) { return; }
    a.clickSound.play();
}

void Audio::startMusic()
{
    AudioState& a = audioInstance();
    if (!a.ready || !a.musicReady) { return; }
    if (a.music.getStatus() == sf::Music::Playing) { return; }

    // Starting fresh (rather than resuming a pause) begins at the highlight.
    if (a.music.getStatus() == sf::Music::Stopped)
    {
        a.music.play();
        a.music.setPlayingOffset(sf::seconds(MUSIC_START_SECONDS));
        return;
    }
    a.music.play();
}

void Audio::stopMusic()
{
    AudioState& a = audioInstance();
    if (!a.ready || !a.musicReady) { return; }
    if (a.music.getStatus() == sf::Music::Playing)
    {
        a.music.pause();
    }
}

bool Audio::musicPlaying()
{
    AudioState& a = audioInstance();
    return a.ready && a.musicReady && a.music.getStatus() == sf::Music::Playing;
}

void Audio::setEffectsVolume(float v)
{
    AudioState& a = audioInstance();
    a.hoverSound.setVolume(v * 0.69f);
    a.clickSound.setVolume(v);
}

void Audio::setMusicVolume(float v)
{
    audioInstance().music.setVolume(v);
}

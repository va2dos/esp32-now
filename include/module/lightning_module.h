#pragma once

#include <FastLED.h>
#include "constants/pins.h"

namespace module
{

    enum class ColorIndex
    {
        White = 0,
        Purple = 1,
        RemotePlay = 2,
        Red = 3,
    };

    enum class LightEffect
    {
        None,
        Loop,
        Pulse
    };

    class LightningModule
    {
    private:
        CRGB leds[pins::NUM_LEDS];

        struct Color
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        Color colors[4] = {{255, 248, 184}, {237, 5, 207}, {25, 154, 25}, {145, 0, 0}};
        int currentColorIndex = 0;

        // Light turning around
        bool lightsOn = false;
        int pos = 0;
        bool configUpdated = false;
        uint8_t brightness = 0;
        int fadeDirection = 1; // 1 for increasing, -1 for decreasing

        // Effect control
        LightEffect currentEffect = LightEffect::Loop;

        void applyLoopEffect();
        void applyPulseEffect();
        void resetEffect();

    public:
        void begin();
        void setLightsOn(bool on, ColorIndex color = ColorIndex::White);
        void setLightMode(LightEffect effect) { currentEffect = effect; configUpdated = true; } 
        bool isLightsOn() const { return lightsOn; }
        void loop();
    };

} // namespace module
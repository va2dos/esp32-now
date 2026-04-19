#pragma once

#include <FastLED.h>
#include "constants/pins.h"

namespace module
{

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

        Color colors[3] = {{255, 248, 184}, {237, 5, 207}, {25, 154, 25}};
        int currentColorIndex = 0;

        // Light turning around
        bool lightsOn = false;
        int pos = 0;
        bool configUpdated = false;

    public:
        enum class ColorIndex
        {
            White = 0,
            Purple = 1,
            RemotePlay = 2
        };

        void begin();
        void setLightsOn(bool on, ColorIndex color = ColorIndex::White);
        bool isLightsOn() const { return lightsOn; }
        void loop();
    };

} // namespace module
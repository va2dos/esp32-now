#ifndef LIGHTNING_SERVICE_H
#define LIGHTNING_SERVICE_H

#include <FastLED.h>
#include "constants/pins.h"

namespace module {

class LightningModule {
private:
    CRGB leds[pins::NUM_LEDS];

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    Color colors[2] = {{255, 248, 184}, {237, 5, 207}};
    int currentColorIndex = 0;

    // Light turning around
    bool lightsOn = false;
    int pos = 0;

public:
    enum class ColorIndex {
        White = 0,
        Purple = 1
    };

    void begin();
    void setLightsOn(bool on, ColorIndex color = ColorIndex::White);
    bool isLightsOn() const { return lightsOn; }
    void runChaseAnimation();    
};

} // namespace module

#endif // LIGHTNING_SERVICE_H
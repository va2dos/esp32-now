#include "module/lightning_module.h"

namespace module {

void LightningModule::begin() {
    FastLED.addLeds<WS2812B, pins::LED_PIN, GRB>(leds, pins::NUM_LEDS);
    FastLED.setBrightness(80);
    FastLED.clear();
    FastLED.show();
}

void LightningModule::setLightsOn(bool on, ColorIndex color) {
    lightsOn = on;
    currentColorIndex = static_cast<int>(color);
    configUpdated = true;
}

void LightningModule::loop() {
    if (configUpdated) {
        FastLED.clear();
        pos = 0; 
        FastLED.show();
        configUpdated = false;
    }

    if (!lightsOn) return;

    // Fade all LEDs slightly
    for (int i = 0; i < pins::NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(40);
    }

    Color c = colors[currentColorIndex];
    leds[pos].setRGB(c.r, c.g, c.b);

    // Update position
    pos++;

    // Bounce at edges
    if (pos >= pins::NUM_LEDS) {
        pos = 0;
    }

    FastLED.show();
}

} // namespace module
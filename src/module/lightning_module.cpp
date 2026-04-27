#include "module/lightning_module.h"

namespace module
{

    void LightningModule::begin()
    {
        FastLED.addLeds<WS2812B, pins::LED_PIN, GRB>(leds, pins::NUM_LEDS);
        FastLED.setBrightness(80);        
        configUpdated = true;
    }

    void LightningModule::setLightsOn(bool on, ColorIndex color)
    {
        lightsOn = on;
        currentColorIndex = static_cast<int>(color);
        configUpdated = true;
    }

    void LightningModule::applyLoopEffect()
    {
        // Fade all LEDs slightly
        for (int i = 0; i < pins::NUM_LEDS; i++)
        {
            leds[i].fadeToBlackBy(40);
        }

        Color c = colors[currentColorIndex];
        leds[pos].setRGB(c.r, c.g, c.b);

        // Update position
        pos++;

        // Bounce at edges
        if (pos >= pins::NUM_LEDS)
        {
            pos = 0;
        }
    }

    void LightningModule::applyPulseEffect()
    {
        Color c = colors[currentColorIndex];
        for (int i = 0; i < pins::NUM_LEDS; i++)
        {
            leds[i].setRGB((c.r * brightness) / 255, (c.g * brightness) / 255, (c.b * brightness) / 255);
        }

        // Update brightness
        brightness += fadeDirection * 5; // Change this value to adjust pulse speed

        // Reverse direction at limits
        if (brightness >= 255)
        {
            brightness = 255;
            fadeDirection = -1;
        }
        else if (brightness <= 0)
        {
            brightness = 0;
            fadeDirection = 1;
        }
    }

    void LightningModule::resetEffect() {
        pos = 0;
        brightness = 0;
        fadeDirection = 1; 
    }

    void LightningModule::loop()
    {
        if (configUpdated)
        {
            FastLED.clear();
            resetEffect();
            FastLED.show();
            configUpdated = false;
        }

        if (!lightsOn)
            return;

        if (currentEffect == LightEffect::Loop)
        {
            applyLoopEffect();
        }
        else if (currentEffect == LightEffect::Pulse)
        {
            applyPulseEffect();
        }

        FastLED.show();
    }

} // namespace module
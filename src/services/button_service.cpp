#include "constants/pins.h"
#include "services/button_service.h"

namespace services
{

    static const unsigned long DEBOUNCE_MS = 50;

    static Button buttons[BUTTON_COUNT] = {
        {pins::ONOFF_BUTTON_PIN, HIGH, HIGH, 0}};

    void ButtonService::begin()
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            pinMode(buttons[i].pin, INPUT_PULLUP);
            Serial.print("Button initialized on pin ");
            Serial.println(buttons[i].pin);
        }
    }

    bool ButtonService::wasPressed(uint8_t index)
    {
        if (index >= BUTTON_COUNT)
            return false;
        return updateButtonInternal(index);
    }

    bool ButtonService::updateButtonInternal(int index)
    {
        Button &b = buttons[index];
        bool reading = digitalRead(b.pin);

        if (reading != b.lastReading)
        {
            b.lastDebounce = millis();
        }

        if (millis() - b.lastDebounce > DEBOUNCE_MS)
        {
            if (reading != b.state)
            {
                b.state = reading;

                if (b.state == LOW)
                {
                    return true; // button was pressed
                }
            }
        }

        b.lastReading = reading;
        return false;
    }

}
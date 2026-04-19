#include "constants/pins.h"
#include "utils/utils.h"
#include "services/button_service.h"

namespace services
{

    static const unsigned long DEBOUNCE_MS = 50;

    static Button buttons[BUTTON_COUNT] = {
        {pins::ONOFF_BUTTON_PIN, HIGH, HIGH, 0, false}};

    void ButtonService::begin()
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            pinMode(buttons[i].pin, INPUT_PULLUP);
            Serial.print("Button initialized on pin ");
            Serial.println(buttons[i].pin);
        }
    }

    void ButtonService::loop()
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {            
            if (updateButtonInternal(i))
            {
                buttons[i].toggleState = !buttons[i].toggleState;
            }
        }
    }

    bool ButtonService::isOn(uint8_t index)
    {
        if (index >= BUTTON_COUNT)
            return false;
        return buttons[index].toggleState;
    }

    bool ButtonService::updateButtonInternal(int index)
    {
        Button &b = buttons[index];
        bool reading = digitalRead(b.pin);

        if (reading != b.lastReading)
        {
            b.lastDebounce = utils::now_ms();
        }

        if (utils::now_ms() - b.lastDebounce > DEBOUNCE_MS)
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
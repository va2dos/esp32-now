#pragma once

#include <Arduino.h>

namespace services
{

    constexpr uint8_t BUTTON_COUNT = 1;

    enum ButtonIndex : uint8_t
    {
        BTN_ON_OFF = 0
    };

    struct Button
    {
        uint8_t pin;
        bool state;       // stable debounced state
        bool lastReading; // last raw reading
        unsigned long lastDebounce;
    };

    class ButtonService
    {
    public:
        void begin();
        bool wasPressed(uint8_t index);

    private:
        bool updateButtonInternal(int index);
    };

}
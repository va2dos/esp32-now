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
        double lastDebounce;
        bool toggleState; // current on/off state for the toggle button
    };

    class ButtonService
    {
    public:
        void begin();
        void loop();
        bool isOn(uint8_t index);

    private:
        bool updateButtonInternal(int index);
    };

}
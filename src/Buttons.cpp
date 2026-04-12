#include "Buttons.h"
#include "Pins.h"

static const unsigned long DEBOUNCE_MS = 50;

struct Button {
    uint8_t pin;
    bool state;          // stable debounced state
    bool lastReading;    // last raw reading
    unsigned long lastDebounce;
};

static Button buttons[BUTTON_COUNT] = {
    {ONOFF_BUTTON_PIN, HIGH, HIGH, 0}
};

void Buttons_begin() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(buttons[i].pin, INPUT_PULLUP);
        Serial.print("Button initialized on pin "); 
        Serial.println(buttons[i].pin);
    }
}

static bool updateButtonInternal(int index) {
    Button &b = buttons[index];
    bool reading = digitalRead(b.pin);

    if (reading != b.lastReading) {
        b.lastDebounce = millis();
    }

    if (millis() - b.lastDebounce > DEBOUNCE_MS) {
        if (reading != b.state) {
            b.state = reading;

            if (b.state == LOW) {
                return true;   // button was pressed
            }
        }
    }

    b.lastReading = reading;
    return false;
}

bool Buttons_wasPressed(uint8_t index) {
    if (index >= BUTTON_COUNT) return false;
    return updateButtonInternal(index);
}

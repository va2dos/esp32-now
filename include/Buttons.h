#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

#define BUTTON_COUNT 1

enum {
    BTN_ON_OFF = 0
};

void Buttons_begin();
bool Buttons_wasPressed(uint8_t index);

#endif // BUTTONS_H

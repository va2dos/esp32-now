#pragma once

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Arduino.h>

#include "constants/pins.h"

namespace module
{
    class CardModule
    {
    public:
        CardModule();
        void begin();
        String checkForCard();

    private:
        Adafruit_PN532 nfc;
    };

}

#pragma once

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <functional>

#include "constants/pins.h"

namespace module
{
    class CardModule
    {
    public:
        CardModule();
        void begin();
        void loop();

        String checkForCard();

        std::function<void(const String &)> onCardDetected;

    private:
        Adafruit_PN532 nfc;
    };

}

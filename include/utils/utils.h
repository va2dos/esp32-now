#pragma once

#include <Arduino.h>

namespace utils
{

    String uidToHexString(uint8_t *uid, uint8_t uidLength);
    String macToStr(const uint8_t *mac);
}

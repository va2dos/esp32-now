#pragma once

#include <Arduino.h>

namespace utils
{
    // Converts a UID byte array to a hexadecimal string representation (e.g., "04AABBCCDD")
    String uidToHexString(uint8_t *uid, uint8_t uidLength);
    // Converts a MAC address to a human-readable string format (e.g., "AA:BB:CC:DD:EE:FF")
    String macToStr(const uint8_t *mac);
    // Returns current time in milliseconds since epoch
    double now_ms();
}

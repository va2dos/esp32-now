#include "Utils.h"

String uidToHexString(uint8_t *uid, uint8_t uidLength) {
    String hex = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) hex += "0";   // leading zero
        hex += String(uid[i], HEX);
    }
    hex.toUpperCase();
    return hex;
}

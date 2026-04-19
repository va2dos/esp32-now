#include <chrono>
#include "utils/utils.h"
namespace utils
{

    String uidToHexString(uint8_t *uid, uint8_t uidLength)
    {
        String hex = "";
        for (uint8_t i = 0; i < uidLength; i++)
        {
            if (uid[i] < 0x10)
                hex += "0"; // leading zero
            hex += String(uid[i], HEX);
        }
        hex.toUpperCase();
        return hex;
    }

    String macToStr(const uint8_t *mac)
    {
        char buf[18];
        snprintf(buf, sizeof(buf),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2],
                 mac[3], mac[4], mac[5]);
        return String(buf);
    }

    double now_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
}
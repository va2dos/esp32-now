#pragma once

#include <stdint.h>

namespace esp_commands
{

    // Speaker 1 MAC: 00:70:07:8A:21:20

    // Stop Playting
    constexpr const char *STOP_COMMAND = "STOP";

    // Play Command Format: "PLAY-XX-YYY"
    constexpr const char *PLAY_COMMAND = "PLAY"; // We'll check the first 4 chars to identify this command
}
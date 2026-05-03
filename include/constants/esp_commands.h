#pragma once

#include <stdint.h>

namespace esp_commands
{

    // Speaker 1 MAC: 00:70:07:8A:21:20
    // Speaker 2 MAC: 88:57:21:79:1E:04
    // Speaker 3 MAC: 88:57:21:E0:79:60
    // Speaker 4 MAC: 14:08:08:AB:BB:10

    // Stop Playting
    constexpr const char *STOP_COMMAND = "STOP";

    // Play Command Format: "PLAY-XX-YYY"
    constexpr const char *PLAY_COMMAND = "PLAY"; // We'll check the first 4 chars to identify this command
}
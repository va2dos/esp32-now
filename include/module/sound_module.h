#pragma once

#include <DFRobotDFPlayerMini.h>
#include "constants/pins.h"

namespace module
{

    class SoundModule
    {
    private:
        HardwareSerial mp3Serial;
        DFRobotDFPlayerMini dfPlayer;
        const unsigned long DFPLAYER_VOLUME = 20;

    public:
        SoundModule();
        void begin();
        void playTrack(uint8_t folder, uint8_t file);
        void playTrack(uint8_t trackNum);
        void stop();
    };

} // namespace module

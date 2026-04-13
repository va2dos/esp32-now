#include "module/sound_module.h"

namespace module {

SoundModule::SoundModule() : mp3Serial(2) {}

void SoundModule::begin() {
    mp3Serial.begin(9600, SERIAL_8N1, pins::MP3_RX_PIN, pins::MP3_TX_PIN);

    if (!dfPlayer.begin(mp3Serial)) {
        Serial.println("DFPlayer Mini not detected!");
    } else {
        Serial.println("DFPlayer Mini ready.");
        // Prewarm to avoid pop sound
        dfPlayer.volume(0);
        dfPlayer.play(1);
        delay(300);
        dfPlayer.stop();
        dfPlayer.volume(DFPLAYER_VOLUME);
    }
}

void SoundModule::playTrack(uint8_t folder, uint8_t file) {
    dfPlayer.playFolder(folder, file);
}

void SoundModule::playTrack(uint8_t trackNum) {
    dfPlayer.play(trackNum);
}

void SoundModule::stop() {
    dfPlayer.stop();
}

} // namespace module
#include <esp_random.h>

#include "constants/pins.h"
#include "utils/utils.h"

#include "module/sound_module.h"
#include "module/lightning_module.h"

module::LightningModule lightningModule;
module::SoundModule soundController;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 120000;

bool musicMode = false;
unsigned long musicModeStart = 0;

void setup() {
    Serial.begin(115200);

    // LED Setup
    lightningModule.begin();

    // Sound Setup
    soundController.begin();

    Serial.print("Setup complete.");
}

void loop() { 

    if(!musicMode) {
        musicMode = true;
        musicModeStart = utils::now_ms();
        int randomNum = (esp_random() % 6) + 1;

        Serial.println("Random number: " + String(randomNum));
        soundController.playTrack(1, randomNum);
        lightningModule.setLightsOn(musicMode);
    } 
    else if (utils::now_ms() - musicModeStart >= MUSIC_MODE_DURATION_MS) {
        musicMode = false;
        soundController.stop();
        lightningModule.setLightsOn(musicMode);
    }

    lightningModule.loop();

    delay(20);
}

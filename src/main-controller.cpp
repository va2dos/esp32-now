#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>

#include "utils/utils.h"

#include "constants/pins.h"
#include "module/sound_module.h"
#include "module/lightning_module.h"
#include "module/card_module.h"

#include "services/button_service.h"
#include "services/esp_now_dispatcher_service.h"
#include "services/track_service.h"

module::LightningModule lightningModule;
module::SoundModule soundController;
module::CardModule cardModule;

services::ButtonService buttonService;
services::EspNowDispatcherService espNowService;
services::TrackService trackService;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 3000;

// Special mode when band will be scanned
bool musicMode = false;
unsigned long musicModeStart = 0;

// Current track info for remote players
bool remotePlayersTriggered = false;
services::TrackInfo trackInfo;

// Button handling moved to include/Buttons.h and src/Buttons.cpp

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Controller Firmware...");

    // ESP-NOW Setup
    espNowService.begin();

    // Card Setup
    cardModule.begin();

    // LED Setup
    lightningModule.begin();

    // Button Setup
    buttonService.begin();

    // Sound Setup
    soundController.begin();

    Serial.print("Setup complete.");
}

void loop()
{

    if (buttonService.wasPressed(services::ButtonIndex::BTN_ON_OFF))
    {
        Serial.println("Mode button pressed, lightsOn toggled");
        bool newState = !lightningModule.isLightsOn();
        lightningModule.setLightsOn(newState);
        if (!newState)
        {
            soundController.stop();
        }
        espNowService.broadcast("STOP");
    }

    String uid = cardModule.checkForCard();
    if (uid.length() > 0)
    {
        Serial.print("Detected UID: ");
        Serial.println(uid);

        if (lightningModule.isLightsOn() && !musicMode && !remotePlayersTriggered)
        {
            musicMode = true;
            musicModeStart = millis();
            soundController.playTrack(1); // Play track 1
            espNowService.broadcast("STOP");
        }
        else if (!lightningModule.isLightsOn())
        {
            Serial.println("Card detected but lights are off, ignoring.");
        }
        else if (musicMode)
        {
            Serial.println("Card detected but already in music mode, ignoring.");
        }
        else if (remotePlayersTriggered)
        {
            Serial.println("Card detected but remote players are active, ignoring.");
        }
    }

    if (lightningModule.isLightsOn())
    {
        lightningModule.setMusicMode(false);
        if (musicMode)
        {
            if (millis() - musicModeStart >= MUSIC_MODE_DURATION_MS)
            {
                musicMode = false;
                soundController.stop();

                // Trigger remote players to play the track
                services::TrackParseError trackResult = trackService.parsePlayCommand("PLAY-02-001", trackInfo);
                if (trackResult == services::TrackParseError::None)
                {
                    char cmd[16];
                    trackService.buildPlayCommand(cmd, trackInfo.folder, trackInfo.file);
                    espNowService.broadcast(cmd);

                    Serial.println("Starting remote track.");
                    remotePlayersTriggered = true;
                    musicModeStart = millis();
                }
                else
                {
                    Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult)));
                }
            }
            else
            {
                lightningModule.setMusicMode(true);
            }
        }
        lightningModule.runChaseAnimation();
    }

    if (remotePlayersTriggered && millis() - musicModeStart >= trackInfo.duration * 1000UL)
    {
        remotePlayersTriggered = false;
        trackInfo = {0}; // reset track info
        Serial.println("Remote track ended, resetting state.");
    }

    espNowService.loop();

    // Small delay to avoid overwhelming the loop
    delay(20);
}

#include <esp_random.h>
#include <esp_now.h>
#include <WiFi.h>

#include "constants/pins.h"
#include "constants/tracks.h"

#include "module/sound_module.h"
#include "module/lightning_module.h"

#include "services/esp_now_service.h"
#include "services/esp_now_client_service.h"
#include "services/track_service.h"
#include "utils/utils.h"

services::EspNowClientService espNowService;
module::LightningModule lightningModule;
module::SoundModule soundController;
services::TrackService trackService;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 120000;

// Setup for DFPlayer Mini
bool musicMode = false;
bool musicReady = false;
unsigned long musicModeStart = 0;
services::TrackInfo trackInfo;

void handleEspNowMessage(const services::EspNowMessage &msg)
{
    Serial.println("Entering handleEspNowMessage...");

    if (msg.msgType != services::MSG_DATA)
    {
        Serial.println("Received message of unknown type");
        return;
    }

    // Example: interpret msg.text as a command
    if (strcmp(msg.text, tracks::STOP_COMMAND) == 0)
    {
        musicMode = false;
        musicReady = false;
        soundController.stop();
        Serial.println("Received STOP command");
    }
    else if (strncmp(msg.text, "PLAY-", 5) == 0)
    {
        Serial.print("Received PLAY command: ");
        Serial.println(msg.text);

        services::TrackParseError trackResult = trackService.parsePlayCommand(msg.text, trackInfo);
        if (trackResult == services::TrackParseError::None)
        {
            Serial.print("Playing track from command: ");
            Serial.print("Folder: ");
            Serial.print(trackInfo.folder);
            Serial.print(", File: ");
            Serial.print(trackInfo.file);
            Serial.print(", Duration: ");
            Serial.print(trackInfo.duration);
            Serial.print("s, Name: ");
            Serial.println(trackInfo.name);

            musicReady = true;
        }
        else
        {
            Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult)));
        }
    }
    else
    {
        Serial.println("Unknown command");
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Speaker Firmware...");

    espNowService.begin();

    // LED Setup
    lightningModule.begin();

    // Sound Setup
    soundController.begin();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register callback
    espNowService.onMessage([](const services::EspNowMessage &msg)
                            { handleEspNowMessage(msg); });

    Serial.print("Setup complete.");
}

void loop()
{
    if(musicReady)
    {        
        Serial.printf("Playing folder=%u file=%u\n", trackInfo.folder, trackInfo.file);
        soundController.playTrack(trackInfo.folder, trackInfo.file);
        musicMode = true;
        musicModeStart = millis();
        musicReady = false;
        lightningModule.setLightsOn(musicMode);
    }
    else if (musicMode && (millis() - musicModeStart) >= ((trackInfo.duration + 1) * 1000UL))
    {
        Serial.println("Track duration ended, stopping music and resetting state.");
        musicMode = false;
        soundController.stop();
        trackInfo = {}; // Clear track info
        lightningModule.setLightsOn(musicMode);
    }

    lightningModule.runChaseAnimation();
    
    espNowService.loop();

    delay(50);
}

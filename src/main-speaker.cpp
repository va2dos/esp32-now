#include "constants/esp_commands.h"

#include "utils/utils.h"

#include "module/sound_module.h"
#include "module/lightning_module.h"

#include "services/esp_now_client_service.h"
#include "services/track_service.h"
#include "services/state_controller.h"

module::LightningModule lightningModule;
module::SoundModule soundController;

services::EspNowClientService espNowService;
services::TrackService trackService;
services::StateController stateController;

// Current track info for players
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
    if (strcmp(msg.text, esp_commands::STOP_COMMAND) == 0)
    {
        Serial.println("Received STOP command");
        stateController.setState(services::SystemState::Idle);
    }
    else if (strncmp(msg.text, esp_commands::PLAY_COMMAND, 5) == 0)
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

            stateController.setState(services::SystemState::Playing);
        }
        else
        {
            Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult) + " for " + String(msg.text)));
        }
    }
    else
    {
        Serial.println("Unknown command: " + String(msg.text));
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
    espNowService.onMessage = [](const services::EspNowMessage &msg)
    {
        handleEspNowMessage(msg);
    };

    // State Controller Setup
    stateController.onIdleEnter = []()
    {
        Serial.println("State -> Idle");
        soundController.stop();
        lightningModule.setLightsOn(false);
    };

    stateController.onPlayingEnter = []()
    {
        Serial.println("State -> Playing");
        lightningModule.setLightsOn(true);
        soundController.playTrack(trackInfo.folder, trackInfo.file);
    };

    stateController.onOffEnter = []()
    {
        Serial.println("State -> Off");
        soundController.stop();
        lightningModule.setLightsOn(false);
    };

    stateController.setState(services::SystemState::Off);

    Serial.print("Setup complete.");
}

void loop()
{
    espNowService.loop();
    lightningModule.loop();

    if (stateController.getState() == services::SystemState::Playing)
    {
        if (stateController.getElapsedTime() >= trackInfo.duration)
        {
            Serial.println("Track duration ended, stopping music and resetting state.");
            trackInfo = {}; // Clear track info
            stateController.setState(services::SystemState::Idle);
        }
    }

    delay(50);
}

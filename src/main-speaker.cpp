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

std::optional<services::EspNowMessage> incomingMsg; // Buffer for incoming ESP-NOW messages

void handleEspNowMessage(const services::EspNowMessage &msg)
{
    Serial.println("Entering handleEspNowMessage...");

    if (msg.msgType == services::MSG_ANNOUNCE)
    {
        // Connected
        lightningModule.setLightsOn(false);
        stateController.setState(services::SystemState::Idle);
        return;
    }
    else if (msg.msgType != services::MSG_DATA)
    {
        Serial.println("Received message of unknown type");
        return;
    }

    Serial.print("Received ESP-NOW message: ");
    Serial.println(msg.text);
    incomingMsg = msg; // Store the incoming message for processing in the main loop if needed
}

void reset()
{
    incomingMsg.reset();
    lightningModule.setLightsOn(false);
    trackInfo = {}; // Clear track info
    lightningModule.setLightMode(module::LightEffect::Loop);
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Speaker Firmware...");

    espNowService.begin();

    // LED Setup
    lightningModule.begin();
    lightningModule.setLightsOn(true, module::ColorIndex::Red);
    lightningModule.setLightMode(module::LightEffect::Pulse);

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
        // TODO setup color based on Scenario
        lightningModule.setLightsOn(true, module::ColorIndex::White);

        Serial.print("Track info -> Folder:");
        Serial.print(trackInfo.folder);
        Serial.print(", File: ");
        Serial.print(trackInfo.file);
        soundController.playTrack(trackInfo.folder, trackInfo.file);
    };

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

    if (incomingMsg.has_value())
    {
        auto message = incomingMsg.value().text;
        Serial.println("Processing message: " + String(message));

        incomingMsg.reset();

        if (strcmp(message, esp_commands::STOP_COMMAND) == 0)
        {
            Serial.println("STOP command : set state to Idle");
            stateController.setState(services::SystemState::Idle);
        }
        else if (strncmp(message, esp_commands::PLAY_COMMAND, 4) == 0)
        {
            Serial.print("PLAY command : parsing track info");

            services::TrackParseError trackResult = trackService.parsePlayCommand(message, trackInfo);
            if (trackResult == services::TrackParseError::None)
            {
                Serial.print("Track info -> Folder:");
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
                Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult) + " for " + String(message)));
            }
        }
        else
        {
            Serial.println("Unknown command: " + String(message));
        }
    }

    delay(50);
}

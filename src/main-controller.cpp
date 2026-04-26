#include "module/sound_module.h"
#include "module/lightning_module.h"
#include "module/card_module.h"

#include "services/button_service.h"
#include "services/esp_now_dispatcher_service.h"
#include "services/track_service.h"
#include "services/state_controller.h"
#include "services/scenario_service.h"

module::LightningModule lightningModule;
module::SoundModule soundController;
module::CardModule cardModule;

services::ButtonService buttonService;
services::EspNowDispatcherService espNowService;
services::TrackService trackService;
services::StateController stateController;
services::ScenarioService scenarioService;

// Current track info for players, used to determine duration
services::TrackInfo trackInfo;
services::ScenarioType currentScenario = services::ScenarioType::None;

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Controller Firmware...");

    // ESP-NOW Setup
    espNowService.begin();

    // Scenario Service Setup
    scenarioService.init(&trackService, &espNowService);

    // LED Setup
    lightningModule.begin();

    // Button Setup
    buttonService.begin();

    // Music play when rfid detected.
    trackService.parsePlayCommand("PLAY-4-1", trackInfo);

    // Card Setup
    cardModule.begin();

    cardModule.onCardDetected = [](const String &uid)
    {
        Serial.print("cardModule > onCardDetected");
        if (stateController.getState() == services::SystemState::Idle)
        {
            // Map UID to scenario
            currentScenario = scenarioService.mapUidToScenario(uid);
            Serial.printf("Mapped UID %s to scenario %d\n", uid.c_str(), static_cast<int>(currentScenario));

            stateController.setState(services::SystemState::RfidDetected);
        }
    };

    // Sound Setup
    soundController.begin();

    // State Controller Setup
    stateController.onIdleEnter = []()
    {
        Serial.println("State -> Idle");
        espNowService.broadcast("STOP");
        lightningModule.setLightsOn(true, module::LightningModule::ColorIndex::White);
    };

    stateController.onRfidDetected = []()
    {
        Serial.println("State -> onRfidDetected");
        stateController.setState(services::SystemState::Playing);
    };

    stateController.onPlayingEnter = []()
    {
        Serial.println("State -> Playing");
        soundController.playTrack(trackInfo.folder, trackInfo.file);
        lightningModule.setLightsOn(true, module::LightningModule::ColorIndex::Purple);
    };

    stateController.onRemotePlayingEnter = []()
    {
        Serial.println("State -> RemotePlaying");
        // Stop Playing locally
        soundController.stop();

        // Execute scenario on remote players
        if (currentScenario != services::ScenarioType::None)
        {
            scenarioService.executeScenario(currentScenario);
            lightningModule.setLightsOn(true, module::LightningModule::ColorIndex::RemotePlay);
        }
        else
        {
            Serial.println("No scenario mapped for this card, skipping remote play.");
            stateController.setState(services::SystemState::Cooldown);
        }
    };

    stateController.onCooldownEnter = []()
    {
        Serial.println("Remote track ended, resetting state.");

        espNowService.broadcast("STOP");
        lightningModule.setLightsOn(false, module::LightningModule::ColorIndex::White);
        stateController.setState(services::SystemState::Idle);
    };

    stateController.setState(services::SystemState::Off);

    Serial.print("Setup complete.");
}

void loop()
{
    espNowService.loop();
    buttonService.loop();
    lightningModule.loop();

    if (buttonService.isOn(services::ButtonIndex::BTN_ON_OFF) && stateController.getState() == services::SystemState::Off)
    {
        Serial.println("On/Off button pressed, switching to Idle state");
        lightningModule.setLightsOn(true, module::LightningModule::ColorIndex::White);
        stateController.setState(services::SystemState::Idle);
    }
    else if (!buttonService.isOn(services::ButtonIndex::BTN_ON_OFF) && stateController.getState() != services::SystemState::Off)
    {
        Serial.println("On/Off button released, switching to Off state");
        soundController.stop();
        espNowService.broadcast("STOP");
        stateController.setState(services::SystemState::Off);
        lightningModule.setLightsOn(false);
    }

    if (stateController.getState() == services::SystemState::Off)
    {
        // If we're off, skip all other processing
        return;
    }

    cardModule.loop();
    scenarioService.loop();

    if (stateController.getState() == services::SystemState::Playing)
    {
        if (stateController.getElapsedTime() >= trackInfo.duration)
        {
            stateController.setState(services::SystemState::RemotePlaying);
        }
    }

    if (stateController.getState() == services::SystemState::RemotePlaying)
    {
        if (scenarioService.isScenarioComplete())
        {
            stateController.setState(services::SystemState::Cooldown);
        }
    }

    // Small delay to avoid overwhelming the loop
    delay(20);
}

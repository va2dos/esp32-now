#include "utils/utils.h"

#include "module/sound_module.h"
#include "module/lightning_module.h"
#include "module/card_module.h"

#include "services/button_service.h"
#include "services/esp_now_dispatcher_service.h"
#include "services/track_service.h"
#include "services/state_controller.h"

module::LightningModule lightningModule;
module::SoundModule soundController;
module::CardModule cardModule;

services::ButtonService buttonService;
services::EspNowDispatcherService espNowService;
services::TrackService trackService;
services::StateController stateController;

// Current track info for players, used to determine duration
services::TrackInfo trackInfo;

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Controller Firmware...");

    // ESP-NOW Setup
    espNowService.begin();

    // LED Setup
    lightningModule.begin();

    // Button Setup
    buttonService.begin();

    // Card Setup
    cardModule.begin();

    cardModule.onCardDetected = [](const String &uid)
    {
        Serial.print("Detected UID: ");
        Serial.println(uid);
        if (stateController.getState() == services::SystemState::Idle)
        {
            // TODO Pick scenarios based on UID, for now we just trigger music mode for any card
            trackService.parsePlayCommand("PLAY-04-001", trackInfo);

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

        // Trigger remote players
        char cmd[16];
        trackService.buildPlayCommand(cmd, trackInfo.folder, trackInfo.file);
        espNowService.broadcast(cmd);
        lightningModule.setLightsOn(true, module::LightningModule::ColorIndex::RemotePlay);
    };

    stateController.onCooldownEnter = []()
    {
        Serial.println("Remote track ended, resetting state.");

        trackInfo = {}; // reset track info
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

    if (stateController.getState() == services::SystemState::Playing)
    {
        if (stateController.getElapsedTime() >= trackInfo.duration)
        {

            // TODO Randomize track selection based on scenario
            trackService.parsePlayCommand("PLAY-02-001", trackInfo);

            stateController.setState(services::SystemState::RemotePlaying);
        }
    }

    if (stateController.getState() == services::SystemState::RemotePlaying)
    {
        if (stateController.getElapsedTime() >= trackInfo.duration)
        {
            stateController.setState(services::SystemState::Cooldown);
        }
    }   

    // Small delay to avoid overwhelming the loop
    delay(20);
}

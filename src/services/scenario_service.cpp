#include "services/scenario_service.h"
#include "services/esp_now_dispatcher_service.h"
#include "utils/utils.h"
#include "constants/rfid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace services
{
    ScenarioService::ScenarioService()
        : trackService(nullptr), espNowService(nullptr)
    {
        currentConfig = {ScenarioType::None, 0, 0, 0};
    }

    void ScenarioService::init(TrackService *trackSvc, EspNowDispatcherService *espNowSvc)
    {
        trackService = trackSvc;
        espNowService = espNowSvc;
    }

    ScenarioType ScenarioService::mapUidToScenario(const String &uid)
    {
        if (!espNowService)
        {
            Serial.println("ERROR: ScenarioService not initialized");
            return ScenarioType::None;
        }

        int clientCount = espNowService->countClient();
        ScenarioType mappedScenario = ScenarioType::Scenario1; // default fallback

        // Map UID to scenario
        if (uid == String(rfid::BAND_CHIP) && clientCount >= 4) {
            mappedScenario = ScenarioType::Scenario2;
        }
        else if (uid == String(rfid::BAND_YOSHI) && clientCount >= 1) {
            mappedScenario = ScenarioType::Scenario3;
        }
        else if (clientCount == 0) {
            Serial.println("No clients available, cannot run any scenario");
            mappedScenario = ScenarioType::None; // No clients, can't run any scenario
        }
            
        return mappedScenario;
    }

    void ScenarioService::executeScenario(ScenarioType scenario)
    {
        if (!trackService || !espNowService)
        {
            Serial.println("ERROR: ScenarioService not properly initialized");
            return;
        }

        int clientCount = espNowService->countClient();

        currentConfig.type = scenario;
        currentConfig.currentClientIndex = 0;
        currentConfig.lastClientPlayStartTime = 0;

        switch (scenario)
        {
        case ScenarioType::Scenario1:
            executeScenario1();
            break;

        case ScenarioType::Scenario2:
            executeScenario2();
            break;

        case ScenarioType::Scenario3:
            executeScenario3();
            break;

        default:
            currentConfig.type = ScenarioType::None;
            break;
        }
    }

    void ScenarioService::executeScenario1()
    {
        // Random sound from track/folder 1 on a random client
        // Folder 1 has 5 tracks (files 1-5)

        int clientCount = espNowService->countClient();
        if (clientCount == 0)
            return;

        // Pick random track from folder 1
        uint8_t randomTrack = (rand() % trackService->getTrackCount(1)) + 1;

        // Pick random client
        uint8_t randomClientIdx = rand() % clientCount;
        uint8_t *clientMac = espNowService->getClientAtIndex(randomClientIdx);
        
        trackService->buildPlayCommand(currentConfig.cmd, 1, randomTrack);

        currentConfig.scenarioDuration = trackService->getTrackDurationMilliseconds(1, randomTrack);

        Serial.printf("Scenario1: Playing track 1-%d on random client %d\n", randomTrack, randomClientIdx);
        espNowService->sendTo(clientMac, currentConfig.cmd);
        currentConfig.lastClientPlayStartTime = utils::now_ms();
    }

    void ScenarioService::executeScenario2()
    {
        // Play sounds from folder 2, one per client (max 4 clients, 4 tracks)
        // Requires minimum 4 clients

        int clientCount = espNowService->countClient();
        if (clientCount < 4)
            return;

        Serial.printf("Scenario2: Playing 4 tracks from folder 2, one per client\n");

        // Tracks in folder 2: 1-4
        for (int i = 0; i < 4; i++)
        {
            uint8_t *clientMac = espNowService->getClientAtIndex(i);            
            trackService->buildPlayCommand(currentConfig.cmd, 2, i + 1);  // Track i+1

            double trackDuration = trackService->getTrackDurationMilliseconds(2, i + 1);
            if (trackDuration > currentConfig.scenarioDuration)
                currentConfig.scenarioDuration = trackDuration;

            Serial.printf("  Sending track 2-%d to client %d\n", i + 1, i);
            espNowService->sendTo(clientMac, currentConfig.cmd);
        }
        currentConfig.lastClientPlayStartTime = utils::now_ms();
    }

    void ScenarioService::executeScenario3()
    {
        // Folder 3 only has track 1
        trackService->buildPlayCommand(currentConfig.cmd, 3, 1);

        currentConfig.scenarioDuration = trackService->getTrackDurationMilliseconds(3, 1);

        // Sequential playback: sound from folder 3 bounces through all clients one after another
        // Start with first client
        Serial.println("Scenario3: Starting sequential bouncing playback");
        playNextClientInSequence(currentConfig.cmd);
    }

    void ScenarioService::playNextClientInSequence(const char *cmd)
    {
        int clientCount = espNowService->countClient();

        if (currentConfig.currentClientIndex >= clientCount)
        {
            // All clients have played, scenario is complete
            currentConfig.type = ScenarioType::None;
            Serial.println("Scenario3: Sequential playback complete");
            return;
        }

        uint8_t *clientMac = espNowService->getClientAtIndex(currentConfig.currentClientIndex);

        Serial.printf("Scenario3: Playing on client %d/%d\n", currentConfig.currentClientIndex + 1, clientCount);
        espNowService->sendTo(clientMac, cmd);

        // Duration of folder 3 track (Mario coin) is 1 second according to TRACKS_REFS
        // Get the actual duration from TRACKS_REFS
        currentConfig.lastClientPlayStartTime = utils::now_ms();
        currentConfig.currentClientIndex++;
    }

    void ScenarioService::loop()
    {
        if(currentConfig.type == ScenarioType::None) 
            return;

        // Handle scenario 3 sequential playback timing
        double elapsed = (utils::now_ms() - currentConfig.lastClientPlayStartTime);

        // When track finishes, play next client
        if (elapsed >= currentConfig.scenarioDuration)
        {
            if (currentConfig.type == ScenarioType::Scenario3)
            {
                playNextClientInSequence(currentConfig.cmd);
            }
            else {
                // For other scenarios, we can consider them complete after the duration has passed
                currentConfig.type = ScenarioType::None;
            }
        }
    }

    bool ScenarioService::isScenarioComplete() const
    {
        return currentConfig.type == ScenarioType::None;
    }

    ScenarioType ScenarioService::getCurrentScenario() const
    {
        return currentConfig.type;
    }

    int ScenarioService::getAvailableClientCount() const
    {
        if (!espNowService)
            return 0;
        return espNowService->countClient();
    }

}

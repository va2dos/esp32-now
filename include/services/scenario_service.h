#pragma once

#include <Arduino.h>
#include <functional>
#include "esp_now_dispatcher_service.h"
#include "track_service.h"

namespace services
{
    enum class ScenarioType : uint8_t
    {
        None = 0,
        Scenario1 = 1,      // Random sound from folder 1 on random client (min 1 client)
        Scenario2 = 2,      // One sound per client from folder 2 (min 4 clients)
        Scenario3 = 3       // Sequential sound from folder 3 on all clients (bouncing)
    };

    struct ScenarioConfig
    {
        ScenarioType type;
        uint8_t minClientsRequired;
        uint8_t currentClientIndex;  // For scenario 3, tracks which client is currently playing
        char cmd[16]; // Command to send for the scenario (e.g., "PLAY 1-3")
        double lastClientPlayStartTime; // Timestamp when the current client started playing
        double scenarioDuration;
    };

    class ScenarioService
    {
    public:
        ScenarioService();

        // Initialize with dependencies
        void init(TrackService *trackService, EspNowDispatcherService *espNowService);

        // Map UID to scenario (returns ScenarioType)
        ScenarioType mapUidToScenario(const String &uid);

        // Execute scenario based on type
        void executeScenario(ScenarioType scenario);

        // Check if all clients have finished (for async scenario 3)
        bool isScenarioComplete() const;

        // Must be called periodically to handle sequential playback
        void loop();

        // Get current scenario
        ScenarioType getCurrentScenario() const;
        int getAvailableClientCount() const;

    private:
        TrackService *trackService;
        EspNowDispatcherService *espNowService;
        ScenarioConfig currentConfig;

        // Scenario implementations
        void executeScenario1();  // Random sound on random client
        void executeScenario2();  // One sound per client
        void executeScenario3();  // Sequential bouncing sound

        // Scenario 3 helpers
        void playNextClientInSequence(const char *cmd);
    };

}

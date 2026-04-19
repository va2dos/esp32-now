#pragma once

#include <functional>
#include "esp_now_service.h"
namespace services
{

    typedef std::function<void(const EspNowMessage &)> ClientMessageCallback;

    class EspNowClientService : public EspNowService
    {
    public:
        EspNowClientService();

        bool begin() override;
        void loop() override;      

        // Allows external code to register a callback for incoming messages
        std::function<void(const EspNowMessage &)> onMessage;

    protected:
        void handleMessage(const uint8_t *mac, const EspNowMessage &msg) override;
        void sendAnnouncement();

    private:        
        static const unsigned long ANNOUNCE_INTERVAL = 3000; // ms

    };

}

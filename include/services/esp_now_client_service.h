#pragma once

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

        void sendAnnouncement();

        // Allows external code to register a callback for incoming messages
        void onMessage(ClientMessageCallback cb);

    protected:
        void handleMessage(const uint8_t *mac, const EspNowMessage &msg) override;

    private:
        unsigned long lastAnnounce;
        static const unsigned long ANNOUNCE_INTERVAL = 3000; // ms

        ClientMessageCallback callback = nullptr;
    };

}

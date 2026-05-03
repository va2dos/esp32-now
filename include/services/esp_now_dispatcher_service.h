#pragma once

#include "esp_now_service.h"

namespace services
{

    struct ClientInfo
    {
        uint8_t mac[6];
        double lastSeen;
    };

    class EspNowDispatcherService : public EspNowService
    {
    public:
        EspNowDispatcherService();

        bool begin() override;
        void loop() override;

        void broadcast(const char *text);
        void sendTo(const uint8_t *mac, const char *text);
        void sendACK(const uint8_t *mac);

        int countClient() const;
        uint8_t* getClientAtIndex(int index);

    protected:
        void handleMessage(const uint8_t *mac, const EspNowMessage &msg) override;

    private:
        static const int MAX_CLIENTS = 20;
        static const unsigned long CLIENT_TIMEOUT = 60000; // ms

        ClientInfo clients[MAX_CLIENTS];
        int clientCount;

        int findClientIndex(const uint8_t *mac) const;
        bool addClient(const uint8_t *mac);
        void updateLastSeen(const uint8_t *mac);
        void pruneStale();
    };

}

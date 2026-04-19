#pragma once

#include <chrono>
#include <esp_now.h>
#include <WiFi.h>

#include "utils/utils.h"

namespace services
{

    enum MsgType
    {
        MSG_ANNOUNCE = 1,
        MSG_DATA = 2
    };

    typedef struct EspNowMessage
    {
        uint8_t msgType;
        uint8_t mac[6];
        char text[128];
    } EspNowMessage;

    class EspNowService
    {
    public:
        EspNowService();
        virtual ~EspNowService() = default;

        virtual bool begin();
        virtual void loop();

    protected:
        virtual void handleMessage(const uint8_t *mac, const EspNowMessage &msg) = 0;
        void sendRaw(const uint8_t *mac, const EspNowMessage &msg);

        // For timing state durations, if needed
        double startTime = 0;
        void resetTimer() { startTime = utils::now_ms(); }        
        double getElapsedTime() const { return utils::now_ms() - startTime; }

    private:
        static void onReceiveStatic(const uint8_t *mac, const uint8_t *data, int len);
        static EspNowService *s_instance;
    };
}

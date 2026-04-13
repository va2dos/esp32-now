#include "services/esp_now_service.h"
#include "utils/utils.h"

namespace services
{

    EspNowService *EspNowService::s_instance = nullptr;

    EspNowService::EspNowService()
    {
        s_instance = this;
    }

    bool EspNowService::begin()
    {
        bool wifiSuccess = WiFi.mode(WIFI_STA);
        if (!wifiSuccess)
        {
            Serial.println("Failed to set WiFi mode");
            return false;
        }

        if (esp_now_init() != ESP_OK)
        {
            Serial.println("ESP-NOW init failed");
            return false;
        }

        esp_now_register_recv_cb(EspNowService::onReceiveStatic);
        Serial.print("ESP-NOW started, MAC: ");
        Serial.println(WiFi.macAddress());

        return true;
    }

    void EspNowService::loop()
    {
        // Base does nothing; subclasses may override
    }

    void EspNowService::sendRaw(const uint8_t *mac, const EspNowMessage &msg)
    {
        esp_err_t result = esp_now_send(mac, (const uint8_t *)&msg, sizeof(msg));
        if (result != ESP_OK)
        {
            Serial.println(utils::macToStr(mac));
            Serial.print("ESP-NOW send error: ");
            Serial.println(result);
        }
    }

    void EspNowService::onReceiveStatic(const uint8_t *mac, const uint8_t *data, int len)
    {
        if (!s_instance)
            return;
        if (len < (int)sizeof(EspNowMessage))
            return;

        EspNowMessage msg;
        memcpy(&msg, data, sizeof(EspNowMessage));
        s_instance->handleMessage(mac, msg);
    }

}
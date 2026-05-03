#include "services/esp_now_client_service.h"

namespace services
{
    EspNowClientService::EspNowClientService(){}

    // Main controller 00:4B:12:F2:44:14
    uint8_t controllerMAC[] = {0x00, 0x4B, 0x12, 0xF2, 0x44, 0x14};

    bool EspNowClientService::begin()
    {
        if (!EspNowService::begin())
        {
            return false;
        }

        Serial.println("Adding dispatcher as peer...");
        esp_now_peer_info_t peer{};
        memcpy(peer.peer_addr, controllerMAC, 6);
        peer.channel = 0;
        peer.encrypt = false;
        esp_err_t result = esp_now_add_peer(&peer);
        if (result != ESP_OK)
        {
            Serial.println("Failed to add dispatcher peer");
            return false;
        }
        
        sendAnnouncement();

        return true;
    }

    void EspNowClientService::loop()
    {
        sendAnnouncement();
    }

    void EspNowClientService::sendAnnouncement()
    {
        if(getElapsedTime() < ANNOUNCE_INTERVAL)
            return;

        EspNowMessage msg{};
        msg.msgType = MSG_ANNOUNCE;
        memcpy(msg.mac, controllerMAC, 6);

        WiFi.macAddress(msg.mac);
        msg.text[0] = '\0';

        sendRaw(controllerMAC, msg);

        resetTimer();
    }

    void EspNowClientService::handleMessage(const uint8_t *mac, const EspNowMessage &msg)
    {
        Serial.print("Client received message from dispatcher: ");
        Serial.println(msg.text);

        if (onMessage)
        {
            onMessage(msg);
        }
    }
}
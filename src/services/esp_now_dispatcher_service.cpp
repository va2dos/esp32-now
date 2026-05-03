#include "utils/utils.h"
#include "services/esp_now_dispatcher_service.h"

namespace services
{

    EspNowDispatcherService::EspNowDispatcherService()
        : clientCount(0) {}

    bool EspNowDispatcherService::begin()
    {
        if (!EspNowService::begin())
            return false;
        clientCount = 0;
        return true;
    }

    void EspNowDispatcherService::loop()
    {
        pruneStale();
    }

    int EspNowDispatcherService::findClientIndex(const uint8_t *mac) const
    {
        for (int i = 0; i < clientCount; i++)
        {
            if (memcmp(clients[i].mac, mac, 6) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    bool EspNowDispatcherService::addClient(const uint8_t *mac)
    {
        if (clientCount >= MAX_CLIENTS)
            return false;

        if (findClientIndex(mac) != -1)
            return false;

        esp_now_peer_info_t peer{};
        memcpy(peer.peer_addr, mac, 6);
        peer.channel = 0;
        peer.encrypt = false;

        if (esp_now_add_peer(&peer) == ESP_OK)
        {
            memcpy(clients[clientCount].mac, mac, 6);
            clients[clientCount].lastSeen = utils::now_ms();
            clientCount++;

            Serial.print("Dispatcher registered client: ");
            for (int i = 0; i < 6; i++)
            {
                Serial.printf("%02X", mac[i]);
                if (i < 5)
                    Serial.print(":");
            }
            Serial.println();
            return true;
        }

        Serial.println("Failed to add peer");
        return false;
    }

    void EspNowDispatcherService::updateLastSeen(const uint8_t *mac)
    {
        int idx = findClientIndex(mac);
        if (idx >= 0)
        {
            clients[idx].lastSeen = utils::now_ms();
        }
    }

    void EspNowDispatcherService::pruneStale()
    {
        double now = utils::now_ms();
        int i = 0;
        while (i < clientCount)
        {
            if (now - clients[i].lastSeen > CLIENT_TIMEOUT)
            {
                Serial.print("Removing stale client: ");
                for (int j = 0; j < 6; j++)
                {
                    Serial.printf("%02X", clients[i].mac[j]);
                    if (j < 5)
                        Serial.print(":");
                }
                Serial.println();

                esp_now_del_peer(clients[i].mac);

                clients[i] = clients[clientCount - 1];
                clientCount--;
            }
            else
            {
                i++;
            }
        }
    }

    void EspNowDispatcherService::broadcast(const char *text)
    {
        EspNowMessage msg{};
        msg.msgType = MSG_DATA;
        WiFi.macAddress(msg.mac);
        strncpy(msg.text, text, sizeof(msg.text));
        msg.text[sizeof(msg.text) - 1] = '\0';

        for (int i = 0; i < clientCount; i++)
        {
            sendRaw(clients[i].mac, msg);
        }

        Serial.print("Dispatcher broadcast: ");
        Serial.println(text);
    }

    void EspNowDispatcherService::sendTo(const uint8_t *mac, const char *text)
    {
        EspNowMessage msg{};
        msg.msgType = MSG_DATA;
        WiFi.macAddress(msg.mac);
        strncpy(msg.text, text, sizeof(msg.text));
        msg.text[sizeof(msg.text) - 1] = '\0';

        sendRaw(mac, msg);

        Serial.print("Dispatcher sent to one client: ");
        Serial.println(text);
    }

    void EspNowDispatcherService::sendACK(const uint8_t *mac)
    {
        EspNowMessage msg{};
        msg.msgType = MSG_ACK;
        WiFi.macAddress(msg.mac);
        msg.text[0] = '\0';

        sendRaw(mac, msg);
    }

    int EspNowDispatcherService::countClient() const
    {
        return clientCount;
    }

    uint8_t *EspNowDispatcherService::getClientAtIndex(int index)
    {
        return clients[index].mac;
    }

    // Protected

    void EspNowDispatcherService::handleMessage(const uint8_t *mac, const EspNowMessage &msg)
    {
        if (msg.msgType == MSG_ANNOUNCE)
        {
            bool added = addClient(msg.mac);
            updateLastSeen(msg.mac);
            Serial.println("Dispatcher received ANNOUNCE");

            Serial.print("New client added, sending ACK to:");
            Serial.println(utils::macToStr(msg.mac));
            sendACK(msg.mac);
        }
        else if (msg.msgType == MSG_DATA)
        {
            Serial.print("Dispatcher received MSG_DATA from client: ");
            Serial.println(msg.text);
        }
    }
}
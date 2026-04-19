#include "module/card_module.h"
#include "utils/utils.h"

namespace module
{

    CardModule::CardModule() : nfc(pins::PN532_SS) {}

    void CardModule::begin()
    {
        nfc.begin();
        uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata)
        {
            Serial.println("Didn't find PN532 board");
            while (1)
                ; // halt
        }
        Serial.println("Found PN532 with firmware version: ");
        Serial.println((versiondata >> 24) & 0xFF, DEC);
        nfc.SAMConfig();
        Serial.println("Waiting for an NFC card...");
    }

    void CardModule::loop()
    {
        auto uidString = checkForCard();
        if (uidString != "")
        {
            Serial.println("Card scanned : " + String(uidString));
            if (onCardDetected)
                onCardDetected(uidString);
        }
    }

    String CardModule::checkForCard()
    {
        uint8_t uid[7];
        uint8_t uidLength;

        bool success = nfc.readPassiveTargetID(
            PN532_MIFARE_ISO14443A,
            uid,
            &uidLength,
            50 // non-blocking timeout
        );

        if (!success)
        {
            return ""; // no card detected
        }

        return utils::uidToHexString(uid, uidLength);
    }
}
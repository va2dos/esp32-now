#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <esp_now.h>
#include <WiFi.h>

#include "utils/utils.h"

#include "constants/pins.h"

#include "services/button_service.h"
#include "services/esp_now_dispatcher_service.h"
#include "services/track_service.h"

services::ButtonService buttonService;
services::EspNowDispatcherService espNowService;
services::TrackService trackService;

Adafruit_PN532 nfc(pins::PN532_SS);

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 3000;

HardwareSerial mp3Serial(2); // UART2
DFRobotDFPlayerMini dfPlayer;
// Adafruit_PN532 nfc(Wire);

CRGB leds[pins::NUM_LEDS];

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
Color colors[] = {{255, 248, 184}, {237, 5, 207}};
int currentColorIndex = 0;

// Light turning around
bool lightsOn = false;
int pos = 0;

// Special mode when band will be scanned
bool musicMode = false;
unsigned long musicModeStart = 0;

// Current track info for remote players
bool remotePlayersTriggered = false;
services::TrackInfo trackInfo;

// Button handling moved to include/Buttons.h and src/Buttons.cpp

String checkForCard()
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

void runChaseAnimation()
{
    // Fade all LEDs slightly
    for (int i = 0; i < pins::NUM_LEDS; i++)
    {
        leds[i].fadeToBlackBy(40);
    }

    Color c = colors[currentColorIndex];
    leds[pos].setRGB(c.r, c.g, c.b);

    // Update position
    pos++;

    // Bounce at edges
    if (pos >= pins::NUM_LEDS)
    {
        pos = 0;
    }

    FastLED.show();
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Controller Firmware...");

    // ESP-NOW Setup
    espNowService.begin();

    // NFC Setup
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

    // LED Setup
    FastLED.addLeds<WS2812B, pins::LED_PIN, GRB>(leds, pins::NUM_LEDS);
    FastLED.setBrightness(80);
    FastLED.clear();
    FastLED.show();

    // Button Setup
    buttonService.begin();

    // MP3 Player Setup
    mp3Serial.begin(9600, SERIAL_8N1, pins::MP3_RX_PIN, pins::MP3_TX_PIN); // RX=16, TX=17

    if (!dfPlayer.begin(mp3Serial))
    {
        Serial.println("DFPlayer Mini not detected!");
    }
    else
    {
        Serial.println("DFPlayer Mini ready.");
        // Prewarm to avoid pop sound
        dfPlayer.volume(0);
        dfPlayer.play(1);
        delay(300);
        dfPlayer.stop();
        dfPlayer.volume(20); // Volume range: 0–30
    }

    Serial.print("Setup complete.");
}

void loop()
{

    if (buttonService.wasPressed(services::ButtonIndex::BTN_ON_OFF))
    {
        Serial.println("Mode button pressed, lightsOn toggled");
        lightsOn = !lightsOn;
        if (!lightsOn)
        {
            dfPlayer.stop();
            FastLED.clear();
            FastLED.show();
        }
        espNowService.broadcast("STOP");
    }

    String uid = checkForCard();
    if (uid.length() > 0)
    {
        Serial.print("Detected UID: ");
        Serial.println(uid);

        if (lightsOn && !musicMode && !remotePlayersTriggered)
        {
            musicMode = true;
            musicModeStart = millis();
            dfPlayer.play(1); // Play track 1
            espNowService.broadcast("STOP");
        }
        else if (!lightsOn)
        {
            Serial.println("Card detected but lights are off, ignoring.");
        }
        else if (musicMode)
        {
            Serial.println("Card detected but already in music mode, ignoring.");
        }
        else if (remotePlayersTriggered)
        {
            Serial.println("Card detected but remote players are active, ignoring.");
        }
    }

    if (lightsOn)
    {
        currentColorIndex = 0;
        if (musicMode)
        {
            if (millis() - musicModeStart >= MUSIC_MODE_DURATION_MS)
            {
                musicMode = false;
                dfPlayer.stop();

                // Trigger remote players to play the track
                services::TrackParseError trackResult = trackService.parsePlayCommand("PLAY-02-001", trackInfo);
                if (trackResult == services::TrackParseError::None)
                {
                    char cmd[16];
                    trackService.buildPlayCommand(cmd, trackInfo.folder, trackInfo.file);
                    espNowService.broadcast(cmd);

                    Serial.println("Starting remote track.");
                    remotePlayersTriggered = true;
                    musicModeStart = millis();
                }
                else
                {
                    Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult)));
                }
            }
            else
            {
                currentColorIndex = 1;
            }
        }
        runChaseAnimation();
    }

    if (remotePlayersTriggered && millis() - musicModeStart >= trackInfo.duration * 1000UL)
    {
        remotePlayersTriggered = false;
        trackInfo = {0}; // reset track info
        Serial.println("Remote track ended, resetting state.");
    }

    espNowService.loop();

    // Small delay to avoid overwhelming the loop
    delay(20);
}

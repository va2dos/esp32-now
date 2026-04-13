#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <esp_random.h>
#include <esp_now.h>
#include <WiFi.h>

#include "constants/pins.h"
#include "constants/tracks.h"

#include "services/esp_now_service.h"
#include "services/esp_now_client_service.h"

#include "services/track_service.h"
#include "utils/utils.h"

services::EspNowClientService espNowService;
services::TrackService trackService;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 120000;

// Setup for DFPlayer Mini
const unsigned long DFPLAYER_VOLUME = 20; // Volume range: 0–30
HardwareSerial mp3Serial(2);              // UART2
DFRobotDFPlayerMini dfPlayer;
bool musicMode = false;
bool musicReady = false;
unsigned long musicModeStart = 0;
services::TrackInfo trackInfo;

// Setup for LEDs
CRGB leds[pins::NUM_LEDS];
struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
Color colors[] = {{255, 248, 184}, {237, 5, 207}};
int currentColorIndex = 0;
int pos = 0;

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
        // Serial.println("Anim: Reached end, bouncing back.");
    }

    FastLED.show();
}

void handleEspNowMessage(const services::EspNowMessage &msg)
{
    Serial.println("Entering handleEspNowMessage...");

    if (msg.msgType != services::MSG_DATA)
    {
        Serial.println("Received message of unknown type");
        return;
    }

    // Example: interpret msg.text as a command
    if (strcmp(msg.text, tracks::STOP_COMMAND) == 0)
    {
        musicMode = false;
        musicReady = false;
        dfPlayer.stop();
        Serial.println("Received STOP command");
    }
    else if (strncmp(msg.text, "PLAY-", 5) == 0)
    {
        Serial.print("Received PLAY command: ");
        Serial.println(msg.text);

        services::TrackParseError trackResult = trackService.parsePlayCommand(msg.text, trackInfo);
        if (trackResult == services::TrackParseError::None)
        {
            Serial.print("Playing track from command: ");
            Serial.print("Folder: ");
            Serial.print(trackInfo.folder);
            Serial.print(", File: ");
            Serial.print(trackInfo.file);
            Serial.print(", Duration: ");
            Serial.print(trackInfo.duration);
            Serial.print("s, Name: ");
            Serial.println(trackInfo.name);

            musicReady = true;
        }
        else
        {
            Serial.println("Parsed track command, result: " + String(static_cast<int>(trackResult)));
        }
    }
    else
    {
        Serial.println("Unknown command");
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting Speaker Firmware...");

    espNowService.begin();

    // LED Setup
    FastLED.addLeds<WS2812B, pins::LED_PIN, GRB>(leds, pins::NUM_LEDS);
    FastLED.setBrightness(80);
    FastLED.clear();

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
        dfPlayer.playFolder(1, 1);
        delay(300);
        dfPlayer.stop();
        dfPlayer.volume(DFPLAYER_VOLUME);
    }

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register callback
    espNowService.onMessage([](const services::EspNowMessage &msg)
                            { handleEspNowMessage(msg); });

    Serial.print("Setup complete.");
}

void loop()
{

    if (musicMode)
    {
        runChaseAnimation();
    }
    else
    {
        FastLED.clear();
        FastLED.show();
        pos = 0;
    }

    if(musicReady)
    {        
        Serial.printf("Playing folder=%u file=%u\n", trackInfo.folder, trackInfo.file);
        dfPlayer.playFolder(trackInfo.folder, trackInfo.file);
        musicMode = true;
        musicModeStart = millis();
        musicReady = false;
    }
    else if (musicMode && (millis() - musicModeStart) >= ((trackInfo.duration + 1) * 1000UL))
    {
        Serial.println("Track duration ended, stopping music and resetting state.");
        musicMode = false;
        dfPlayer.stop();
        trackInfo = {}; // Clear track info
    }

    espNowService.loop();

    delay(50);
}

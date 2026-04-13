#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <esp_random.h>

#include "constants/pins.h"

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 120000;

// Setup for DFPlayer Mini
HardwareSerial mp3Serial(2);  // UART2
DFRobotDFPlayerMini dfPlayer;
bool musicMode = false;
unsigned long musicModeStart = 0;

// Setup for LEDs
CRGB leds[pins::NUM_LEDS];
struct Color { uint8_t r; uint8_t g; uint8_t b; };
Color colors[] = { {255, 248, 184}, {237, 5, 207} };
int currentColorIndex = 0;
int pos = 0;

void runChaseAnimation() {
    // Fade all LEDs slightly
    for (int i = 0; i < pins::NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(40);
    }

    Color c = colors[currentColorIndex]; 
    leds[pos].setRGB(c.r, c.g, c.b);

    // Update position
    pos++;

    // Bounce at edges
    if (pos >= pins::NUM_LEDS) {
        pos = 0;
    }

    FastLED.show();
}

void setup() {
    Serial.begin(115200);

    // LED Setup
    FastLED.addLeds<WS2812B, pins::LED_PIN, GRB>(leds, pins::NUM_LEDS); // GRB order for WS2812B
    FastLED.setBrightness(80);
    FastLED.clear();

    // MP3 Player Setup
    mp3Serial.begin(9600, SERIAL_8N1, pins::MP3_RX_PIN, pins::MP3_TX_PIN); // RX=16, TX=17

    if (!dfPlayer.begin(mp3Serial)) {
        Serial.println("DFPlayer Mini not detected!");
    } else {
        Serial.println("DFPlayer Mini ready.");
        // Prewarm to avoid pop sound
        dfPlayer.volume(0);
        dfPlayer.playFolder(1, 1);
        delay(300);
        dfPlayer.stop();
        dfPlayer.volume(30); // Volume range: 0–30
    }

    Serial.print("Setup complete.");
}

void loop() {

    runChaseAnimation();

    if(!musicMode) {
        musicMode = true;
        musicModeStart = millis();
        int randomNum = (esp_random() % 6) + 1;

        Serial.println("Random number: " + String(randomNum));
        dfPlayer.playFolder(1, randomNum);
    } 
    else if (millis() - musicModeStart >= MUSIC_MODE_DURATION_MS) {
        musicMode = false;
        dfPlayer.stop();
    }

    delay(20);
}

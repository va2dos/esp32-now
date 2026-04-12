#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h> 
#include <Adafruit_PN532.h> 
#include <esp_now.h>
#include <WiFi.h>

#include "Pins.h"
#include "Buttons.h"
#include "Utils.h"
#include "EspNowMessage.h"

Adafruit_PN532 nfc(PN532_SS);

const unsigned long DEBOUNCE_MS = 50;
const unsigned long MUSIC_MODE_DURATION_MS = 3000;

HardwareSerial mp3Serial(2);  // UART2
DFRobotDFPlayerMini dfPlayer;
// Adafruit_PN532 nfc(Wire);

CRGB leds[NUM_LEDS];

struct Color { uint8_t r; uint8_t g; uint8_t b; };
Color colors[] = { {255, 248, 184}, {237, 5, 207} };
int currentColorIndex = 0;

// Light turning around
bool lightsOn = false;
int pos = 0;

// Special mode when band will be scanned
bool musicMode = false;
unsigned long musicModeStart = 0;

// Esp-Now List of receivers
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;
EspNowMessage espNowMessage;

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Button handling moved to include/Buttons.h and src/Buttons.cpp

String checkForCard() {
    uint8_t uid[7];
    uint8_t uidLength;

    bool success = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        50   // non-blocking timeout
    );

    if (!success) {
        return "";   // no card detected
    }

    return uidToHexString(uid, uidLength);
}

void runChaseAnimation() {
    // Fade all LEDs slightly
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(40);
    }

    Color c = colors[currentColorIndex]; 
    leds[pos].setRGB(c.r, c.g, c.b);

    // Update position
    pos++;

    // Bounce at edges
    if (pos >= NUM_LEDS) {
        pos = 0;
    }

    FastLED.show();
}

void setup() {
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    // NFC Setup
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("Didn't find PN532 board");
        while (1); // halt
    }
    Serial.println("Found PN532 with firmware version: ");
    Serial.println((versiondata >> 24) & 0xFF, DEC);
    nfc.SAMConfig(); 
    Serial.println("Waiting for an NFC card...");

    // LED Setup
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(80);
    FastLED.clear();
    FastLED.show();

    // Button Setup
    Buttons_begin();

    // MP3 Player Setup
    mp3Serial.begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN); // RX=16, TX=17

    if (!dfPlayer.begin(mp3Serial)) {
        Serial.println("DFPlayer Mini not detected!");
    } else {
        Serial.println("DFPlayer Mini ready.");
        // Prewarm to avoid pop sound
        dfPlayer.volume(0);
        dfPlayer.play(1);
        delay(300);
        dfPlayer.stop();
        dfPlayer.volume(20); // Volume range: 0–30
    }

    // Esp-Now Register peer
    espNowMessage.id = 0;
    esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
   
    //Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    } 

    Serial.print("Setup complete.");
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendEspNowMessage(const char* message) {
    strcpy(espNowMessage.text, message);
    espNowMessage.id ++;  // Increment ID for each message  
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&espNowMessage, sizeof(espNowMessage));
    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }
}

void loop() {
    if (Buttons_wasPressed(BTN_ON_OFF)) {
        Serial.println("Mode button pressed, lightsOn toggled");
        lightsOn = !lightsOn;
        if (!lightsOn) {
            dfPlayer.stop();
            FastLED.clear();
            FastLED.show();
        }
    }

    String uid = checkForCard(); 
    if (uid.length() > 0) {
        Serial.print("Detected UID: "); 
        Serial.println(uid);

        if(lightsOn && !musicMode) {
            musicMode = true;
            musicModeStart = millis();
            dfPlayer.play(1);  // Play track 1
        }
    }

    if (lightsOn) {
        currentColorIndex = 0;
        if(musicMode) {
            if (millis() - musicModeStart >= MUSIC_MODE_DURATION_MS) { 
                musicMode = false;
                dfPlayer.stop();
                sendEspNowMessage("PLAY-01-01");
            } else {
                currentColorIndex = 1;
            }
        }
        runChaseAnimation();
    }

    delay(20);
}

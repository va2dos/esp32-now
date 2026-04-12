#ifndef PINS_H
#define PINS_H

// ---------------------------------------------------------
// LED STRIP (WS2812B)
// ---------------------------------------------------------
#define LED_PIN             5       // Data pin for LED strip
#define NUM_LEDS            16      // Number of LEDs in the strip

// ---------------------------------------------------------
// BUTTONS
// ---------------------------------------------------------
#define ONOFF_BUTTON_PIN    25      // Toggle lights Switch File Jaunes

// ---------------------------------------------------------
// DFPLAYER MINI (UART2)
// ---------------------------------------------------------
#define MP3_RX_PIN          16      // ESP32 RX2  <- DFPlayer TX
#define MP3_TX_PIN          17      // ESP32 TX2  -> DFPlayer RX

// ---------------------------------------------------------
// PN532 NFC READER (SPI MODE)
// ---------------------------------------------------------
#define PN532_SCK           18      // SPI Clock
#define PN532_MISO          19      // SPI MISO
#define PN532_MOSI          23      // SPI MOSI
#define PN532_SS            4       // SPI Chip Select (SS)

#endif // PINS_H

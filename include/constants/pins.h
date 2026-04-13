#pragma once

namespace pins
{
    // ---------------------------------------------------------
    // I2C (for OLED Display)
    // ---------------------------------------------------------
    constexpr int I2C_SDA_PIN = 21; // I2C Data pin
    constexpr int I2C_SCL_PIN = 22; // I2C Clock pin

    // ---------------------------------------------------------
    // LED STRIP (WS2812B)
    // ---------------------------------------------------------
    constexpr int LED_PIN = 5;   // Data pin for LED strip
    constexpr int NUM_LEDS = 16; // Number of LEDs in the strip

    // ---------------------------------------------------------
    // BUTTONS
    // ---------------------------------------------------------
    constexpr int ONOFF_BUTTON_PIN = 25; // Toggle lights Switch File Jaunes

    // ---------------------------------------------------------
    // DFPLAYER MINI (UART2)
    // ---------------------------------------------------------
    constexpr int MP3_RX_PIN = 16; // ESP32 RX2  <- DFPlayer TX
    constexpr int MP3_TX_PIN = 17; // ESP32 TX2  -> DFPlayer RX

    // ---------------------------------------------------------
    // PN532 NFC READER (SPI MODE)
    // ---------------------------------------------------------
    constexpr int PN532_SCK = 18;  // SPI Clock
    constexpr int PN532_MISO = 19; // SPI MISO
    constexpr int PN532_MOSI = 23; // SPI MOSI
    constexpr int PN532_SS = 4;    // SPI Chip Select (SS)

}
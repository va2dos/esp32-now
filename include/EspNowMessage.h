#ifndef ESP_NOW_MESSAGE_H
#define ESP_NOW_MESSAGE_H

#include <Arduino.h>

typedef struct EspNowMessage {
  uint id;  
  char text[32];
} EspNowMessage;

#endif // ESP_NOW_MESSAGE_H

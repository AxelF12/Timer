#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP32Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

struct TimerData {
  const char* name;
  int duration;
};

void readData(JsonDocument& doc);
void addData(JsonDocument& doc, const char *newName, int newDuration);
TimerData scrollData(JsonDocument& doc, int index);
void removeData(JsonDocument& doc, int index);
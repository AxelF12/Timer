#pragma once
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include <ArduinoJson.h>


// Set these to your desired credentials.


void printLocalTime();
void timeavailable(struct timeval *t);
void wifiapsetup();
void gettime(int timeArray[]);
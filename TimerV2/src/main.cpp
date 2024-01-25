
#include <Arduino.h>
#include "config.h"
#include "data.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <LittleFS.h>
#include "wlan.h"
#include <Adafruit_NeoPixel.h>
// Import required libraries

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define NEOPIXEL_PIN 14 // Pin, an dem die Neopixel angeschlossen sind
#define NUMPIXELS 8     // Anzahl der Neopixel-LEDs
//LED`s auf dem Board
#define LED_1 17
#define LED_2 5
#define LED_3 18

ESP32Encoder encoder;
#define outputA 35 // CLK PIN
#define outputB 32 // DT PIN
#define outputSW 33 // SW Pin

int counter = 0;
int lastCount = 0; // Verändert, damit das Display beim Start aktualisiert wird
 // Anzahl der verfügbaren Optionen anpassen
int subOption = 0; // Variable für Unteroptionen
bool buttonstate3 = false;
bool buttonstate4 = false;
bool switchPressed = false; // Schalterstatus verfolgen
int SEITE = 1;
int SEITEOLD = 1;
int settimer;
int timeArray[6];
bool timerstate = false;
static int timer_start = 1;
static int timer_end = 1;
bool timer_ENDE = false;
int percent = 0;

#define BUTTON1 2
#define BUTTON2 4
#define BUTTON3 16

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // globale Variable display

StaticJsonDocument<2048> doc;

void setup() {
  Serial.begin(9600);
  
  pinMode(outputSW, INPUT_PULLUP);
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(outputA, INPUT);
  pinMode(BUTTON1, INPUT_PULLDOWN);
  pinMode(BUTTON2, INPUT_PULLDOWN);
  pinMode(BUTTON3, INPUT_PULLDOWN);
  encoder.attachSingleEdge(outputA, outputB);
/*
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
long double timerPercent(int& var) {
  long double x = millis() - timer_start;
  int timerDuration = var - timer_start;
  int percentage = x / timerDuration * 100;

  // Steuere die Neopixel-Farbe basierend auf dem Timer-Fortschritt
 /* if (percentage >= 0 && percentage <= 20) {
    pixels.fill(255, 0, 0); // Rot
  } else if (percentage > 20 && percentage <= 40) {
    pixels.fill(0, 0, 255); // Blau
  } else if (percentage > 40 && percentage <= 60) {
    pixels.fill(255, 255, 0); // Gelb
  } else if (percentage > 60 && percentage <= 80) {
    pixels.fill(255, 165, 0); // Orange
  } else {
    pixels.fill(0, 255, 0); // Grün
  }

  pixels.show(); // Zeige die Neopixel-Farbe an
  return x / timerDuration * 100;
}

*/
  // Initialisiere LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Fehler beim Initialisieren von LittleFS");
    return;
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 Verteilung fehlt"));
    while (true);
  }
 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  

  wifiapsetup();
  readData(doc);
  serializeJson(doc, Serial);
}

void printDateScreen(){
  char dateString[20];  // Adjust the size based on your date format
  char timeString[20];

  // Create a formatted date and time string
  sprintf(dateString, "%02d/%02d/%04d", timeArray[2], timeArray[1], timeArray[0]);
  sprintf(timeString, "%02d:%02d:%02d", timeArray[3], timeArray[4], timeArray[5]);
  
  // Clear the display
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(dateString);

  // Print the time string on the next line
  display.setCursor(80, 0);  // Adjust the Y position based on your font size and display size
  display.print(timeString);
}

bool timerstart(int& timer_duration){
  if(timerstate == false){
    timerstate = true;
    timer_start = millis();
    timer_end = timer_start+timer_duration*60000;
  }
  if (millis() > timer_end)
  {
    return true;
  }else {
    return false;
  }
}

long double timerPercent(int& var){
  long double x = millis() - timer_start;
  
  return x / (var-timer_start)*100;
}

int mainScreen(int timeArray[]){
  printDateScreen();
  display.setCursor(0,10);
  display.println("Hauptmenue");
  display.setCursor(0,30);
  display.println("Liste  Timer  Favorit");

  // Display the content on the OLED
  display.display();
  return 1;
}

int listScreen(){
  printDateScreen();
  TimerData timer = scrollData(doc, counter);

  settimer = timer.duration;
  display.setCursor(0,30);
  display.print(timer.name);
  display.setCursor(100,40);
  display.println(timer.duration);
  
  display.println(doc["timer"].size());
  display.println(counter);
  // Display the content on the OLED
  display.display();
  return 2;
}

int setTimerScreen(int counter){
  printDateScreen();

  display.setCursor(0,30);
  display.print("waehle Timer Dauer: ");
  display.setCursor(60, 40);
  display.print(counter);
  // Display the content on the OLED
  display.display();
  settimer = counter;
  return 3;
}

int timerScreen(){
  printDateScreen();
  display.setCursor(0,30);
  display.print("Timer Dauer: ");
  display.setCursor(60, 40);
  display.print(settimer);
  display.setCursor(60, 50);
  display.print(percent);
  // Display the content on the OLED
  display.display();
  return 4;
}

int addtimerScreen(){
  printDateScreen();
  display.setCursor(0,30);
  display.print("Timer Dauer einstellen: ");
  display.setCursor(60, 40);
  display.print(counter);
  display.display();
  return 5;
}

int encoderWert(int optionen) {
  int encoderValue = encoder.getCount();

  if (encoderValue > lastCount) {
    counter++;
  } else if (encoderValue < lastCount) {
    counter--;
  }

  // Begrenze den Wertebereich
  if (counter >= optionen) {
    counter = 0;
  } else if (counter < 0) {
    counter = optionen - 1;
  }
  encoder.setCount(counter);

  if (counter != lastCount) {
    Serial.println(optionen);
    lastCount = counter;
  }

  return counter;  // Gib die aktuelle Counter-Zahl zurück
}



void loop() {
  Serial.println("------------------------------------------");
  readData(doc);
  //addData(doc,"Neuer Eintrag", counter);
  //serializeJson(doc, Serial);
  
  Serial.print(SEITE);
  
  Serial.print("\n");
  Serial.print("\n");

 // pixels.begin();
  //display.clearDisplay();
 // display.setTextSize(1);
  // display.setTextColor(WHITE);
  
  bool button01 = digitalRead(BUTTON1);
  bool button02 = digitalRead(BUTTON2);
  bool button03 = digitalRead(BUTTON3);
  
  if(SEITE == 1 && button01 == true){
    SEITE = 2;
  }else if (SEITE == 1 && button02 == true){
    SEITE = 3;
  }else if ((SEITE == 2 || SEITE == 3) && button01 == true){
    SEITE = 1;
  }else if ((SEITE == 2 || SEITE == 3) && digitalRead(outputSW) == false){
    SEITE = 4;
  }else if (SEITE == 2 && button02 == true){
    SEITE = 5;
  }else if (SEITE == 4 && (button01 == true || timer_ENDE == true)){
    timer_ENDE = false;
    SEITE = 1;
  }else if (SEITE == 5 && (button01 == true)){
    SEITE = 2;
  }
  

  if(SEITE != SEITEOLD){
    Serial.print("HALLLDJFOSJFG");
    encoder.setCount(0);
    lastCount = 0;
    counter = 0;
    timerstate = false;
    timer_end = 0;
  }

  if (SEITE==1){
    SEITE = mainScreen(timeArray);
  }else if (SEITE==2){
    counter = encoderWert(doc["timer"].size());
    bool currentbuttonstate3 = button03;
    if (buttonstate3 == true && currentbuttonstate3 == false){
      Serial.print("NEIN LÖSCH MICH NICHT!!!!");
      removeData(doc, counter);
    }
    buttonstate3 = currentbuttonstate3;
    
    
  
    SEITE = listScreen();
  }else if (SEITE==3){
    counter = encoderWert(120);
    SEITE = setTimerScreen(counter);
  }else if (SEITE==4){
    timer_ENDE = timerstart(settimer);
    percent = timerPercent(timer_end);
    SEITE = timerScreen();
  }else if (SEITE==5){
    //Serial.print("ist weg wie dein vater!!!!");
    counter = encoderWert(120);
    bool currentbuttonstate4 = button02;
    if (buttonstate4 == true && currentbuttonstate4 == false){
      addData(doc,"Neuer Eintrag",counter);
      SEITE = 2;
    }else{
      buttonstate4 = currentbuttonstate4;
      SEITE = addtimerScreen();
    }
    
  }
  
  
  gettime(timeArray);
  Serial.println(timer_end);
  Serial.println(percent);
  /*Serial.println(timeArray[0]);
  Serial.print("Month: ");
  Serial.println(timeArray[1]);
  Serial.print("Day: ");
  Serial.println(timeArray[2]);
  Serial.print("Hour: ");
  Serial.println(timeArray[3]);
  Serial.print("Minute: ");
  Serial.println(timeArray[4]);
  Serial.print("Second: ");
  Serial.println(timeArray[5]);
  Serial.println(millis());*/
  SEITEOLD = SEITE;
  sleep(0.74);
}
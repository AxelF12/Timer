#include "data.h"
// Definieren Sie den Dateinamen
const char *filename = "/data.json";



void readData(JsonDocument& doc) {
  // Öffnen Sie die Datei zum Lesen
  File file = LittleFS.open(filename, "r");

  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Lesen");
    return;
  }

  DeserializationError error = deserializeJson(doc, file);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

}

void addData(JsonDocument& doc, const char *newName, int newDuration) {
  // Öffnen Sie die Datei zum Lesen und Schreiben (read and write)
  File file = LittleFS.open(filename, "r+");

  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Schreiben");
    return;
  }

  DeserializationError error = deserializeJson(doc, file);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    file.close();
    return;
  }

  // Erstellen Sie ein neues JSON-Objekt für den neuen Eintrag
  JsonObject newEntry = doc["timer"].createNestedObject();
  newEntry["name"] = newName;
  newEntry["duration"] = newDuration;

  // Schreibe den aktualisierten Inhalt in die Datei
  file.seek(0);
  serializeJson(doc, file);

  // Schließe die Datei
  file.close();
}

TimerData scrollData(JsonDocument& doc, int index){
  JsonArray timerArray = doc["timer"].as<JsonArray>();
  // Access the element at the current index
  JsonObject timerItem = timerArray[index].as<JsonObject>();
  const char *name = timerItem["name"];
  int duration = timerItem["duration"];
  Serial.println(name);

  if (index >= timerArray.size()) {
    index = 0;
  }
  TimerData timer;
  timer.name= name;
  timer.duration = duration;
  return timer;
}

void removeData(JsonDocument& doc, int index){
  JsonArray timerArray = doc["timer"].as<JsonArray>();
  // Access the element at the current index

  if (index >= 0 && index < timerArray.size()) {
    // Remove the timer set at the specified index
    timerArray.remove(index);
  } else {
    // Handle the case where the index is out of bounds
    Serial.println("Invalid index to delete");
  }

  File file = LittleFS.open(filename, "r+");
  file.seek(0);
  serializeJson(doc, file);

  // Schließe die Datei
  file.close();
}


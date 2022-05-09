#include <Arduino.h>
#include <ArduinoJson.h>
#include <MIDI.h>
#include "debounce-button.h"
#include "patchlist.h"

#define CS_PIN 10

#define INC_PIN 9
#define DEC_PIN 8

const char* test2 = "Hest";

Patchlist patchlist;




void setup() {
	Serial.begin(9600);
  patchlist.begin();
}

int lastState = HIGH;

Button incBtn(INC_PIN);
Button decBtn(DEC_PIN);

void loop() {
  if (incBtn.getState() == LOW) {
    patchlist.next();
  }
  if (decBtn.getState() == LOW) {
    patchlist.prev();
  }
}
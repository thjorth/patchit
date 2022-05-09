#include <Arduino.h>
const int BUTTON_DEBOUNCE_DELAY = 50;   // the debounce time; increase if the output flickers

class Button {
  public:
    Button(int pin);
    int getState();
  private:
    int _pin = -1;
    int _lastSteadyState = LOW;       // the previous steady state from the input pin
    int _lastFlickerableState = LOW;  // the previous flickerable state from the input pin
    int _currentState;                // the current reading from the input pin
    unsigned long _lastDebounceTime = 0;  // the last time the output pin was toggled
};

Button::Button(int pin) {
  _pin = pin;
  pinMode(_pin, INPUT_PULLUP);
}

int Button::getState() {
  int returnVal = -1;
  _currentState = digitalRead(_pin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (_currentState != _lastFlickerableState) {
    // reset the debouncing timer
    _lastDebounceTime = millis();
    // save the the last flickerable state
    _lastFlickerableState = _currentState;
  }

  if ((millis() - _lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (_lastSteadyState == HIGH && _currentState == LOW) {
      returnVal = LOW;
    }     

    // save the the last steady state
    _lastSteadyState = _currentState;
  } 
  return returnVal;
}



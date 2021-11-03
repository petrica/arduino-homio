#include <Arduino.h>

static ArduinoMock* arduinoMock = NULL;
ArduinoMock* arduinoMockInstance() {
  if(!arduinoMock) {
    arduinoMock = new testing::StrictMock<ArduinoMock>();
  }
  return arduinoMock;
}

void releaseArduinoMock() {
  if(arduinoMock) {
    delete arduinoMock;
    arduinoMock = NULL;
  }
}

void delay(time_t a) {
  assert (arduinoMock != NULL);
  arduinoMock->delay(a);
}
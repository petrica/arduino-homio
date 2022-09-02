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

int main(int argc, char** argv) {
    std::cout << "Running main() from gmock_main.cc\n";
    // Since Google Mock depends on Google Test, InitGoogleMock() is
    // also responsible for initializing Google Test.  Therefore there's
    // no need for calling testing::InitGoogleTest() separately.
    testing::InitGoogleMock(&argc, argv);

    if (RUN_ALL_TESTS())
    ;

    return 0;
}
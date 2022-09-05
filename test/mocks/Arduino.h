/**
 * Arduino mock header
 */
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <ctime>
#include <gmock/gmock.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t boolean;
typedef uint8_t byte;

void setup(void);
void loop(void);

void delay(time_t);
int min(int a, int b);

#ifdef __cplusplus
} // extern "C"
#endif

class ArduinoMock {
    public:
        ArduinoMock() {
            
        }

        MOCK_METHOD(void, delay, (int));
};
ArduinoMock* arduinoMockInstance();
void releaseArduinoMock();

#endif // ARDUINO_
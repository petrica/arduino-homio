#ifndef _NRFLite_h_
#define _NRFLite_h_

#include <Arduino.h>
#include <gmock/gmock.h>

enum Bitrates { BITRATE2MBPS, BITRATE1MBPS, BITRATE250KBPS };
enum SendType { REQUIRE_ACK, NO_ACK };

class NRFLite {
    public:
        NRFLite() {}
        virtual uint8_t send(uint8_t toRadioId, void *data, uint8_t length) = 0;
        virtual uint8_t hasAckData() = 0;
        virtual void readData(void *data) = 0;
};

class NRFLiteMock: public NRFLite {
    public:
        NRFLiteMock(): NRFLite() {};
        // MOCK_METHOD(uint8_t, send, (uint8_t toRadioId, void *data, uint8_t length), (override));
        MOCK_METHOD(uint8_t, send, (uint8_t toRadioId, void *data, uint8_t length), (override));
        MOCK_METHOD(uint8_t, hasAckData, (), (override));
        MOCK_METHOD(void, readData, (void *data), (override));
};

#endif
#include <hub/hub.h>
#include <utils.h>

namespace homio 
{

    Hub::Hub(uint8_t deviceId, uint8_t cePin, uint8_t csnPin) {
        deviceId_ = deviceId;
        address_[0] = deviceId;
        addressData_[0] = deviceId + 1;
        cePin_ = cePin;
        csnPin_ = csnPin;
    }

    void Hub::begin() {
        Serial.println(F("Hub begin"));

        radio_ = new RF24(cePin_, csnPin_);

        if (!radio_->begin()) {
            hasFatalError_ = true;
            Serial.println(F("Could not initiate connection with the radio chip."));
            return;
        }

        radio_->setDataRate(RF24_250KBPS);
        radio_->setChannel(channel_);
        radio_->setCRCLength(RF24_CRC_8);
        radio_->enableAckPayload();
        radio_->openReadingPipe(0, address_);
        radio_->openReadingPipe(1, addressData_);
        radio_->startListening();
        radio_->printDetails();
    }

    bool Hub::sendRawCommand(const Command& command, std::vector<uint8_t>& buffer) {
        bool sent = false;
        switch (command.type) {
            case CommandType::LOCK_DELIVER:
                    sent = radio_->writeAckPayload(0, &buffer.front(), buffer.size());
                break;
        }

        return sent;
    }

    void Hub::tick() {
        uint8_t pipe;
        uint8_t bytes;
        std::vector<uint8_t> buffer(HOMIO_NRF24_BUFFER_SIZE);
        Command command{};
        Datapoint datapoint{};
        if (radio_->available(&pipe)) {
            Serial.println(pipe);
            switch (pipe) {
                case 0:
                        Serial.println(F("Hub received pipe 0 command"));
                        bytes = radio_->getPayloadSize();
                        radio_->read(&buffer.front(), bytes);
                        handleRawCommand(buffer, command);
                        switch (command.type)
                        {
                            case CommandType::LOCK_REQUEST:
                                    sendCommand(Command{
                                        .type = CommandType::LOCK_DELIVER, 
                                        .fromAddress = deviceId_, 
                                        .toAddress = command.fromAddress
                                    });
                                break;
                        }
                    break;
                case 1:
                        Serial.println(F("Hub received pipe 1 command"));
                        bytes = radio_->getPayloadSize();
                        Serial.print("Size (bytes): ");
                        Serial.println(bytes);
                        radio_->read(&buffer.front(), bytes);
                        handleRawCommand(buffer, command);
                        Serial.print("Command type: ");
                        Serial.println((uint8_t)command.type);
                        Serial.print("From address: ");
                        Serial.println(command.fromAddress);
                        Serial.print("To address: ");
                        Serial.println(command.toAddress);
                        if (command.type == CommandType::DATAPOINT_REPORT) {
                            unpackDatapoint(command.payload, datapoint);
                            debugDatapoint(datapoint);
                        }
                    break;
            }
            
        }
    }
};
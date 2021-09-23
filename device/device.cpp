#include <device/device.h>

namespace homio
{
    Device::Device(uint8_t deviceId, uint8_t hubId, uint8_t cePin, uint8_t csnPin): Component() {
        deviceId_ = deviceId;
        hubId_ = hubId;
        cePin_ = cePin;
        csnPin_ = csnPin;
    }   

    void Device::begin() {
        Serial.println(F("Device started."));

        radio_ = new NRFLite(Serial);
        if (!radio_->init(deviceId_, cePin_, csnPin_, NRFLite::BITRATE250KBPS, channel_)) {
            hasFatalError_ = true;
            Serial.println(F("Could not initiate connection with the radio chip."));
            return;
        }

        radio_->printDetails();

        state_ = DeviceState::IDLE;
    }

    bool Device::sendRawCommand(const Command& command, std::vector<uint8_t>& raw) {
        Serial.println("Send Raw Command.");
        bool sent = false;
        switch (command.type) {
            case CommandType::LOCK_REQUEST:
                    if (radio_->send(hubId_, &raw[0], raw.size())) {
                        sent = true;
                        Serial.println(F("Sent lock request"));
                        while (radio_->hasAckData())
                        {
                            std::vector<uint8_t> buffer(HOMIO_NRF24_BUFFER_SIZE);
                            radio_->readData(&buffer.front());
                            Serial.println(F("Received LOCK_REQUEST ACK"));
                            Command ackCommand{};
                            handleRawCommand(buffer, ackCommand);
                            Serial.println(ackCommand.toAddress);
                            // Lock intended for this device
                            if (ackCommand.toAddress == deviceId_) {
                                state_ = DeviceState::DATA_SEND;
                            }
                        }
                    }
                break;
            case CommandType::DATAPOINT_REPORT:
            case CommandType::HEARTBEAT:
                    sent = radio_->send(hubId_ + 1, &raw[0], raw.size());
                break;
            default:
                Serial.println(F("Command type not supported."));
        }

        return sent;
    }

    void Device::tick() {
        switch (state_)
        {
            case DeviceState::IDLE:
                if (getCommandQueueSize() > 0) {
                    state_ = DeviceState::LOCK_REQUEST;
                }
                break;
            case DeviceState::LOCK_REQUEST:
                Serial.println(F("Entering LOCK_REQUEST device state."));
                if (!sendCommand(Command{.type = CommandType::LOCK_REQUEST, .fromAddress = deviceId_, .toAddress = hubId_})) {
                    Serial.println(F("Failed to send LOCK_REQUEST to hub."));
                    state_ = DeviceState::IDLE;
                }
                break;
            case DeviceState::DATA_SEND:
                    Serial.println(F("Entering DATA_SEND device state."));
                    processCommandQueue();
                    state_ = DeviceState::IDLE;
                break;
            default:
                break;
        }

        delay(1000);
    }

    void Device::setDatapointValue(uint8_t id, uint32_t value) {
        std::vector<uint8_t> buffer;
        if (packDatapointValue(id, value, buffer)) {
            Command command = {
                .type = CommandType::DATAPOINT_REPORT,
                .fromAddress = deviceId_,
                .toAddress = hubId_,
                .payload = buffer
            };

            queueCommand(command);
        }
    }
}

#include <Device.h>

namespace Homio {

    Device::Device(DeviceTransport *deviceTransport): 
        transport_(deviceTransport) {

    }

    void Device::tick() {
        transport_->tick();
    }

    bool Device::sendDatapoint(const uint8_t datapointId) {
        Datapoint *datapoint = getDatapoint(datapointId);

        if (datapoint) {
            return transport_->writeDatapoint(datapoint);
        }

        return false;   
    }

    void Device::addDatapoint(Datapoint *datapoint) {
        datapoints_[datapointsCount_] = datapoint;
        datapointsCount_ += 1;
    }

    Datapoint *Device::getDatapoint(uint8_t datapointId) {
        for(uint8_t i = 0; i < datapointsCount_; i++) {
            if (datapoints_[i]->id == datapointId) {
                return datapoints_[i];
            }
        }

        return nullptr;
    }
}
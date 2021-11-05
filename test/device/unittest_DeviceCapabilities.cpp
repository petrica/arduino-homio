#include <gmock/gmock.h>
#include <Component.h>
#include <Matchers.h>
#include <ComponentUnderTest.h>

using namespace ::testing;
using namespace Homio;

class DeviceCapabilitiesTest : public Test {
  public:
    ComponentUnderTest *underTest;
    uint8_t buffer[HOMIO_BUFFER_SIZE - HOMIO_COMMAND_HEADER_SIZE];

  void SetUp() {
    underTest = new ComponentUnderTest();
  }

  void TearDown() {
    delete underTest;
    underTest = nullptr;
  }
};

TEST_F(DeviceCapabilitiesTest, WhenSerializeThenExpectStructLength) {
    DeviceCapabilities expectedCapabilities = {};
    expectedCapabilities.heartbeatInterval = 86400;
    expectedCapabilities.canReceive = true;
    uint8_t expectedLength = 5;

    uint8_t actualLength = underTest->serializeDeviceCapabilities(&expectedCapabilities, buffer);

    ASSERT_THAT(actualLength, Eq(expectedLength));
}

TEST_F(DeviceCapabilitiesTest, WhenSerializeThenUnserializeSuccesfully) {
    DeviceCapabilities actualCapabilities = {};
    DeviceCapabilities expectedCapabilities = {};
    expectedCapabilities.heartbeatInterval = 86400;
    expectedCapabilities.canReceive = true;

    underTest->serializeDeviceCapabilities(&expectedCapabilities, buffer);
    underTest->unserializeDeviceCapabilities(buffer, &actualCapabilities);

    ASSERT_THAT(actualCapabilities, Field(&DeviceCapabilities::heartbeatInterval, Eq(expectedCapabilities.heartbeatInterval)));
    ASSERT_THAT(actualCapabilities, Field(&DeviceCapabilities::canReceive, Eq(expectedCapabilities.canReceive)));
}

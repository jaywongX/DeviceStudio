#include <gtest/gtest.h>
#include "core/devicemanager/device.h"
#include "core/devicemanager/idevice.h"

using namespace DeviceStudio;

// 简单的测试设备类
class TestDevice : public Device {
public:
    TestDevice() : Device(DeviceType::Serial) {
        setName("TestDevice");
    }
    
    bool connect(const QVariantMap& config) override {
        setConfiguration(config);
        setDeviceState(DeviceState::Connected);
        return true;
    }
    
    void disconnect() override {
        setDeviceState(DeviceState::Disconnected);
    }
    
    qint64 send(const QByteArray& data) override {
        lastSentData_ = data;
        return data.size();
    }
    
    QByteArray receive() override {
        return QByteArray::fromHex("010203");
    }
    
    QByteArray lastSentData() const { return lastSentData_; }

private:
    QByteArray lastSentData_;
};

class DeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        device = std::make_unique<TestDevice>();
    }
    
    std::unique_ptr<TestDevice> device;
};

// 测试设备创建
TEST_F(DeviceTest, CreateDevice) {
    EXPECT_EQ(device->deviceName().toStdString(), "TestDevice");
    EXPECT_EQ(device->deviceType(), DeviceType::Serial);
    EXPECT_EQ(device->deviceState(), DeviceState::Disconnected);
}

// 测试设备连接
TEST_F(DeviceTest, Connect) {
    EXPECT_TRUE(device->connect(QVariantMap()));
    EXPECT_EQ(device->deviceState(), DeviceState::Connected);
}

// 测试设备断开
TEST_F(DeviceTest, Disconnect) {
    device->connect(QVariantMap());
    device->disconnect();
    EXPECT_EQ(device->deviceState(), DeviceState::Disconnected);
}

// 测试设备ID
TEST_F(DeviceTest, DeviceId) {
    EXPECT_FALSE(device->deviceId().isEmpty());
    
    // 每个设备的ID应该是唯一的
    auto device2 = std::make_unique<TestDevice>();
    EXPECT_NE(device->deviceId(), device2->deviceId());
}

// 测试数据发送
TEST_F(DeviceTest, SendData) {
    device->connect(QVariantMap());
    QByteArray data = QByteArray::fromHex("01020304");
    qint64 sent = device->send(data);
    EXPECT_EQ(sent, 4);
    EXPECT_EQ(device->lastSentData(), data);
}

// 测试数据接收
TEST_F(DeviceTest, ReceiveData) {
    device->connect(QVariantMap());
    QByteArray received = device->receive();
    EXPECT_EQ(received.size(), 3);
    EXPECT_EQ(static_cast<uint8_t>(received[0]), 0x01);
}

// 测试设备状态信号
TEST_F(DeviceTest, StateSignal) {
    DeviceState lastState = DeviceState::Disconnected;
    QObject::connect(device.get(), &IDevice::stateChanged, [&lastState](DeviceState state) {
        lastState = state;
    });
    
    device->connect(QVariantMap());
    EXPECT_EQ(lastState, DeviceState::Connected);
    
    device->disconnect();
    EXPECT_EQ(lastState, DeviceState::Disconnected);
}



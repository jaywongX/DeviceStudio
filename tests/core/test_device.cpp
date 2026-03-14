#include <gtest/gtest.h>
#include "core/devicemanager/device.h"
#include "core/devicemanager/idevice.h"

using namespace DeviceStudio::Core;

// 简单的测试设备类
class TestDevice : public Device {
public:
    TestDevice(const std::string& name) : Device(name, DeviceType::Serial) {}
    
    bool doConnect() override {
        setConnected(true);
        return true;
    }
    
    bool doDisconnect() override {
        setConnected(false);
        return true;
    }
    
    int sendData(const std::vector<uint8_t>& data) override {
        return static_cast<int>(data.size());
    }
    
    std::vector<uint8_t> receiveData(int timeoutMs) override {
        return {0x01, 0x02, 0x03};
    }
};

class DeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        device = std::make_unique<TestDevice>("TestDevice");
    }
    
    std::unique_ptr<TestDevice> device;
};

// 测试设备创建
TEST_F(DeviceTest, CreateDevice) {
    EXPECT_EQ(device->name(), "TestDevice");
    EXPECT_EQ(device->type(), DeviceType::Serial);
    EXPECT_EQ(device->status(), DeviceStatus::Disconnected);
}

// 测试设备连接
TEST_F(DeviceTest, Connect) {
    EXPECT_TRUE(device->connect());
    EXPECT_EQ(device->status(), DeviceStatus::Connected);
}

// 测试设备断开
TEST_F(DeviceTest, Disconnect) {
    device->connect();
    EXPECT_TRUE(device->disconnect());
    EXPECT_EQ(device->status(), DeviceStatus::Disconnected);
}

// 测试设备信息
TEST_F(DeviceTest, DeviceInfo) {
    device->setInfo("port", "COM1");
    device->setInfo("baudrate", "115200");
    
    EXPECT_EQ(device->getInfo("port"), "COM1");
    EXPECT_EQ(device->getInfo("baudrate"), "115200");
}

// 测试设备描述
TEST_F(DeviceTest, Description) {
    device->setDescription("Test device for unit testing");
    EXPECT_EQ(device->description(), "Test device for unit testing");
}

// 测试设备ID
TEST_F(DeviceTest, DeviceId) {
    EXPECT_FALSE(device->id().empty());
    
    // 每个设备的ID应该是唯一的
    auto device2 = std::make_unique<TestDevice>("TestDevice2");
    EXPECT_NE(device->id(), device2->id());
}

// 测试数据发送
TEST_F(DeviceTest, SendData) {
    device->connect();
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    int sent = device->sendData(data);
    EXPECT_EQ(sent, 4);
}

// 测试数据接收
TEST_F(DeviceTest, ReceiveData) {
    device->connect();
    auto received = device->receiveData(1000);
    EXPECT_EQ(received.size(), 3);
    EXPECT_EQ(received[0], 0x01);
}

// 测试设备状态变化回调
TEST_F(DeviceTest, StatusCallback) {
    DeviceStatus lastStatus = DeviceStatus::Disconnected;
    device->setStatusChangedCallback([&lastStatus](DeviceStatus status) {
        lastStatus = status;
    });
    
    device->connect();
    EXPECT_EQ(lastStatus, DeviceStatus::Connected);
    
    device->disconnect();
    EXPECT_EQ(lastStatus, DeviceStatus::Disconnected);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

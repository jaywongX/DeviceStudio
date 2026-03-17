/**
 * @file test_devicemanager.cpp
 * @brief 设备管理器单元测试
 * @author DeviceStudio Team
 * @date 2026-03-16
 */

#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include "core/devicemanager/devicemanager.h"
#include "core/devicemanager/idevice.h"

using namespace DeviceStudio;

class DeviceManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        deviceManager_ = std::make_shared<DeviceManager>();
    }
    
    void TearDown() override
    {
        deviceManager_->removeAllDevices();
    }
    
    std::shared_ptr<DeviceManager> deviceManager_;
};

TEST_F(DeviceManagerTest, CreateDevice)
{
    // 创建串口设备
    QVariantMap config;
    config["port"] = "COM1";
    config["baudRate"] = 115200;
    
    auto device = deviceManager_->createDevice(DeviceType::Serial, "TestSerial", config);
    EXPECT_TRUE(device != nullptr);
    EXPECT_EQ(device->deviceName(), "TestSerial");
    EXPECT_EQ(device->deviceType(), DeviceType::Serial);
}

TEST_F(DeviceManagerTest, AddDevice)
{
    QVariantMap config;
    auto device = deviceManager_->createDevice(DeviceType::Serial, "AddTest", config);
    
    bool result = deviceManager_->addDevice(device);
    EXPECT_TRUE(result);
    EXPECT_EQ(deviceManager_->deviceCount(), 1);
}

TEST_F(DeviceManagerTest, RemoveDevice)
{
    QVariantMap config;
    auto device = deviceManager_->createDevice(DeviceType::Serial, "RemoveTest", config);
    deviceManager_->addDevice(device);
    
    QString deviceId = device->deviceId();
    bool result = deviceManager_->removeDevice(deviceId);
    EXPECT_TRUE(result);
    EXPECT_EQ(deviceManager_->deviceCount(), 0);
}

TEST_F(DeviceManagerTest, GetDevice)
{
    QVariantMap config;
    auto device = deviceManager_->createDevice(DeviceType::Serial, "GetTest", config);
    deviceManager_->addDevice(device);
    
    QString deviceId = device->deviceId();
    auto retrieved = deviceManager_->getDevice(deviceId);
    EXPECT_TRUE(retrieved != nullptr);
    EXPECT_EQ(retrieved->deviceName(), "GetTest");
}

TEST_F(DeviceManagerTest, GetAllDevices)
{
    QVariantMap config;
    
    auto device1 = deviceManager_->createDevice(DeviceType::Serial, "Device1", config);
    auto device2 = deviceManager_->createDevice(DeviceType::TcpClient, "Device2", config);
    
    deviceManager_->addDevice(device1);
    deviceManager_->addDevice(device2);
    
    auto devices = deviceManager_->getAllDevices();
    EXPECT_EQ(devices.size(), 2);
}

TEST_F(DeviceManagerTest, FindDevicesByType)
{
    QVariantMap config;
    
    auto serial1 = deviceManager_->createDevice(DeviceType::Serial, "Serial1", config);
    auto serial2 = deviceManager_->createDevice(DeviceType::Serial, "Serial2", config);
    auto tcp = deviceManager_->createDevice(DeviceType::TcpClient, "TCP", config);
    
    deviceManager_->addDevice(serial1);
    deviceManager_->addDevice(serial2);
    deviceManager_->addDevice(tcp);
    
    auto serialDevices = deviceManager_->findDevicesByType(DeviceType::Serial);
    EXPECT_EQ(serialDevices.size(), 2);
    
    auto tcpDevices = deviceManager_->findDevicesByType(DeviceType::TcpClient);
    EXPECT_EQ(tcpDevices.size(), 1);
}

TEST_F(DeviceManagerTest, FindDevicesByName)
{
    QVariantMap config;
    
    auto device1 = deviceManager_->createDevice(DeviceType::Serial, "UniqueName", config);
    auto device2 = deviceManager_->createDevice(DeviceType::Serial, "OtherName", config);
    
    deviceManager_->addDevice(device1);
    deviceManager_->addDevice(device2);
    
    auto found = deviceManager_->findDevicesByName("UniqueName");
    EXPECT_EQ(found.size(), 1);
    EXPECT_EQ(found.first()->deviceName(), "UniqueName");
}

TEST_F(DeviceManagerTest, DeviceCount)
{
    EXPECT_EQ(deviceManager_->deviceCount(), 0);
    
    QVariantMap config;
    auto device = deviceManager_->createDevice(DeviceType::Serial, "CountTest", config);
    deviceManager_->addDevice(device);
    
    EXPECT_EQ(deviceManager_->deviceCount(), 1);
}

TEST_F(DeviceManagerTest, RemoveAllDevices)
{
    QVariantMap config;
    
    auto device1 = deviceManager_->createDevice(DeviceType::Serial, "D1", config);
    auto device2 = deviceManager_->createDevice(DeviceType::Serial, "D2", config);
    
    deviceManager_->addDevice(device1);
    deviceManager_->addDevice(device2);
    
    EXPECT_EQ(deviceManager_->deviceCount(), 2);
    
    deviceManager_->removeAllDevices();
    EXPECT_EQ(deviceManager_->deviceCount(), 0);
}

TEST_F(DeviceManagerTest, DeviceSignals)
{
    QVariantMap config;
    auto device = deviceManager_->createDevice(DeviceType::Serial, "SignalTest", config);
    
    QSignalSpy addedSpy(deviceManager_.get(), &DeviceManager::deviceAdded);
    QSignalSpy removedSpy(deviceManager_.get(), &DeviceManager::deviceRemoved);
    
    deviceManager_->addDevice(device);
    EXPECT_EQ(addedSpy.count(), 1);
    
    deviceManager_->removeDevice(device->deviceId());
    EXPECT_EQ(removedSpy.count(), 1);
}

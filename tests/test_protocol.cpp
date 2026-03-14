/**
 * @file test_protocol.cpp
 * @brief 协议引擎单元测试
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include <gtest/gtest.h>
#include "protocol/protocolengine.h"
#include "protocol/protocolparser.h"
#include "protocol/checksum.h"

using namespace DeviceStudio;

class ProtocolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = ProtocolEngine::instance();
    }
    
    void TearDown() override
    {
        engine->unloadAllProtocols();
    }
    
    ProtocolEngine* engine;
};

// ========== 校验和测试 ==========

TEST(ChecksumTest, XorChecksum)
{
    QByteArray data = QByteArray::fromHex("0102030405");
    uint8_t result = ChecksumCalculator::xorChecksum(data);
    EXPECT_EQ(result, 0x01 ^ 0x02 ^ 0x03 ^ 0x04 ^ 0x05);
}

TEST(ChecksumTest, SumChecksum)
{
    QByteArray data = QByteArray::fromHex("0102030405");
    uint8_t result = ChecksumCalculator::sumChecksum(data);
    EXPECT_EQ(result, (0x01 + 0x02 + 0x03 + 0x04 + 0x05) & 0xFF);
}

TEST(ChecksumTest, Crc16Modbus)
{
    // Modbus RTU 示例: 地址01, 功能码03, 寄存器地址0000, 数量000A
    QByteArray data = QByteArray::fromHex("01030000000A");
    uint16_t crc = ChecksumCalculator::crc16Modbus(data);
    // CRC应该是 C5CD (小端序存储为 CD C5)
    EXPECT_EQ(crc, 0xC5CD);
}

TEST(ChecksumTest, Crc8)
{
    QByteArray data = QByteArray::fromHex("0102030405");
    uint8_t crc = ChecksumCalculator::crc8(data);
    EXPECT_NE(crc, 0); // 非零值
}

TEST(ChecksumTest, ToBytesBigEndian)
{
    uint32_t value = 0x12345678;
    QByteArray bytes = ChecksumCalculator::toBytes(value, 4, ByteOrder::BigEndian);
    EXPECT_EQ(bytes.size(), 4);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0x12);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0x34);
    EXPECT_EQ(static_cast<uint8_t>(bytes[2]), 0x56);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 0x78);
}

TEST(ChecksumTest, ToBytesLittleEndian)
{
    uint32_t value = 0x12345678;
    QByteArray bytes = ChecksumCalculator::toBytes(value, 4, ByteOrder::LittleEndian);
    EXPECT_EQ(bytes.size(), 4);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0x78);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0x56);
    EXPECT_EQ(static_cast<uint8_t>(bytes[2]), 0x34);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 0x12);
}

// ========== 协议解析测试 ==========

TEST_F(ProtocolTest, LoadProtocolFromJson)
{
    QString json = R"({
        "protocol_name": "TestProtocol",
        "protocol_version": "1.0",
        "description": "测试协议",
        "frame_header": "AA55",
        "frame_tail": "0D0A",
        "min_length": 10,
        "max_length": 256,
        "checksum": {
            "enabled": true,
            "type": "xor",
            "start_offset": 0,
            "end_offset": -1
        },
        "fields": [
            {
                "name": "header",
                "display_name": "帧头",
                "offset": 0,
                "length": 2,
                "type": "bytes"
            },
            {
                "name": "length",
                "display_name": "长度",
                "offset": 2,
                "length": 1,
                "type": "uint8"
            },
            {
                "name": "data",
                "display_name": "数据",
                "offset": 3,
                "length": 4,
                "type": "uint32",
                "byte_order": "big"
            }
        ]
    })";
    
    ProtocolParser parser;
    bool result = parser.loadFromJson(json);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.protocol().name, "TestProtocol");
    EXPECT_EQ(parser.protocol().fields.size(), 3);
}

TEST_F(ProtocolTest, ParseSimpleFrame)
{
    // 创建简单协议
    QString json = R"({
        "protocol_name": "SimpleProtocol",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {
            "enabled": false
        },
        "fields": [
            {
                "name": "value1",
                "display_name": "值1",
                "offset": 2,
                "length": 1,
                "type": "uint8"
            },
            {
                "name": "value2",
                "display_name": "值2",
                "offset": 3,
                "length": 1,
                "type": "uint8"
            }
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    // 解析数据: AA 55 12 34
    QByteArray data = QByteArray::fromHex("AA551234");
    ParseResult result = parser.parse(data);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getValue("value1").toUInt(), 0x12);
    EXPECT_EQ(result.getValue("value2").toUInt(), 0x34);
}

TEST_F(ProtocolTest, ParseUint16BigEndian)
{
    QString json = R"({
        "protocol_name": "Uint16Test",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [
            {
                "name": "value",
                "display_name": "值",
                "offset": 2,
                "length": 2,
                "type": "uint16",
                "byte_order": "big"
            }
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    // 数据: AA 55 12 34 (大端序 = 0x1234 = 4660)
    QByteArray data = QByteArray::fromHex("AA551234");
    ParseResult result = parser.parse(data);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getValue("value").toUInt(), 0x1234);
}

TEST_F(ProtocolTest, ParseUint16LittleEndian)
{
    QString json = R"({
        "protocol_name": "Uint16Test",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [
            {
                "name": "value",
                "display_name": "值",
                "offset": 2,
                "length": 2,
                "type": "uint16",
                "byte_order": "little"
            }
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    // 数据: AA 55 34 12 (小端序 = 0x1234 = 4660)
    QByteArray data = QByteArray::fromHex("AA553412");
    ParseResult result = parser.parse(data);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getValue("value").toUInt(), 0x1234);
}

TEST_F(ProtocolTest, ParseWithScale)
{
    QString json = R"({
        "protocol_name": "ScaleTest",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [
            {
                "name": "temperature",
                "display_name": "温度",
                "offset": 2,
                "length": 2,
                "type": "uint16",
                "scale": 0.1,
                "unit": "°C"
            }
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    // 数据: AA 55 00 FA (250 * 0.1 = 25.0)
    QByteArray data = QByteArray::fromHex("AA5500FA");
    ParseResult result = parser.parse(data);
    
    EXPECT_TRUE(result.success);
    EXPECT_DOUBLE_EQ(result.getValue("temperature").toDouble(), 25.0);
    EXPECT_TRUE(result.getDisplayValue("temperature").contains("°C"));
}

TEST_F(ProtocolTest, ParseWithValueMap)
{
    QString json = R"({
        "protocol_name": "ValueMapTest",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [
            {
                "name": "status",
                "display_name": "状态",
                "offset": 2,
                "length": 1,
                "type": "uint8",
                "value_map": {
                    "0": "停止",
                    "1": "运行",
                    "2": "暂停",
                    "3": "错误"
                }
            }
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    QByteArray data = QByteArray::fromHex("AA5501");
    ParseResult result = parser.parse(data);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getValue("status").toUInt(), 1);
    EXPECT_EQ(result.getDisplayValue("status"), "运行");
}

// ========== 协议引擎测试 ==========

TEST_F(ProtocolTest, RegisterProtocol)
{
    auto parser = std::make_shared<ProtocolParser>();
    
    ProtocolDefinition def;
    def.name = "TestProtocol";
    def.version = "1.0";
    def.id = "TestProtocol_1.0";
    
    ProtocolField field;
    field.name = "test";
    field.offset = 0;
    field.length = 1;
    def.fields.append(field);
    
    parser->setProtocol(def);
    
    EXPECT_TRUE(engine->registerProtocol(parser));
    EXPECT_TRUE(engine->hasProtocol("TestProtocol"));
    EXPECT_EQ(engine->protocolCount(), 1);
}

TEST_F(ProtocolTest, AutoParse)
{
    // 注册多个协议
    auto parser1 = std::make_shared<ProtocolParser>();
    QString json1 = R"({
        "protocol_name": "Proto1",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [{"name": "data", "offset": 2, "length": 1, "type": "uint8"}]
    })";
    parser1->loadFromJson(json1);
    engine->registerProtocol(parser1);
    
    auto parser2 = std::make_shared<ProtocolParser>();
    QString json2 = R"({
        "protocol_name": "Proto2",
        "protocol_version": "1.0",
        "frame_header": "BB66",
        "min_length": 4,
        "checksum": {"enabled": false},
        "fields": [{"name": "data", "offset": 2, "length": 1, "type": "uint8"}]
    })";
    parser2->loadFromJson(json2);
    engine->registerProtocol(parser2);
    
    // 自动解析
    QByteArray data1 = QByteArray::fromHex("AA551234");
    QList<ParseResult> results1 = engine->autoParse(data1);
    EXPECT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0].protocolName, "Proto1");
    
    QByteArray data2 = QByteArray::fromHex("BB66ABCD");
    QList<ParseResult> results2 = engine->autoParse(data2);
    EXPECT_EQ(results2.size(), 1);
    EXPECT_EQ(results2[0].protocolName, "Proto2");
}

// ========== 数据打包测试 ==========

TEST_F(ProtocolTest, PackData)
{
    QString json = R"({
        "protocol_name": "PackTest",
        "protocol_version": "1.0",
        "frame_header": "AA55",
        "frame_tail": "0D0A",
        "checksum": {"enabled": false},
        "fields": [
            {"name": "value1", "offset": 2, "length": 1, "type": "uint8"},
            {"name": "value2", "offset": 3, "length": 1, "type": "uint8"}
        ]
    })";
    
    ProtocolParser parser;
    ASSERT_TRUE(parser.loadFromJson(json));
    
    QMap<QString, QVariant> values;
    values["value1"] = 0x12;
    values["value2"] = 0x34;
    
    QByteArray packed = parser.pack(values);
    
    EXPECT_EQ(packed.size(), 6);
    EXPECT_EQ(static_cast<uint8_t>(packed[0]), 0xAA);
    EXPECT_EQ(static_cast<uint8_t>(packed[1]), 0x55);
    EXPECT_EQ(static_cast<uint8_t>(packed[2]), 0x12);
    EXPECT_EQ(static_cast<uint8_t>(packed[3]), 0x34);
    EXPECT_EQ(static_cast<uint8_t>(packed[4]), 0x0D);
    EXPECT_EQ(static_cast<uint8_t>(packed[5]), 0x0A);
}

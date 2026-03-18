/**
 * @file checksum.h
 * @brief 校验和计算工具类
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QByteArray>
#include <cstdint>
#include "protocoldefinition.h"

namespace DeviceStudio {

/**
 * @brief 校验和计算器
 * 
 * 提供多种校验和计算方法
 */
class ChecksumCalculator
{
public:
    /**
     * @brief 计算校验和
     * @param data 数据
     * @param type 校验类型
     * @return 校验和值
     */
    static QByteArray calculate(const QByteArray& data, ChecksumType type);
    
    /**
     * @brief 验证校验和
     * @param data 数据(包含校验和)
     * @param type 校验类型
     * @param checksumOffset 校验和偏移
     * @param checksumLength 校验和长度
     * @return 是否验证通过
     */
    static bool verify(const QByteArray& data, ChecksumType type, 
                      int checksumOffset, int checksumLength = 1);
    
    // ========== 具体校验算法 ==========
    
    /**
     * @brief XOR校验(异或)
     * @param data 数据
     * @return 校验值
     */
    static uint8_t xorChecksum(const QByteArray& data);
    
    /**
     * @brief SUM校验(求和取低字节)
     * @param data 数据
     * @return 校验值
     */
    static uint8_t sumChecksum(const QByteArray& data);
    
    /**
     * @brief SUM16校验(求和取低字)
     * @param data 数据
     * @return 校验值
     */
    static uint16_t sum16Checksum(const QByteArray& data);
    
    /**
     * @brief CRC8校验
     * @param data 数据
     * @param poly 多项式(默认0x07)
     * @param init 初始值(默认0x00)
     * @return 校验值
     */
    static uint8_t crc8(const QByteArray& data, uint8_t poly = 0x07, uint8_t init = 0x00);
    
    /**
     * @brief CRC16校验(CCITT)
     * @param data 数据
     * @return 校验值
     */
    static uint16_t crc16(const QByteArray& data);
    
    /**
     * @brief CRC16 Modbus校验
     * @param data 数据
     * @return 校验值
     */
    static uint16_t crc16Modbus(const QByteArray& data);
    
    /**
     * @brief CRC32校验
     * @param data 数据
     * @return 校验值
     */
    static uint32_t crc32(const QByteArray& data);
    
    /**
     * @brief 将校验和转换为字节数组
     * @param value 校验值
     * @param length 字节长度
     * @param byteOrder 字节序
     * @return 字节数组
     */
    static QByteArray toBytes(uint64_t value, int length, ByteOrder byteOrder = ByteOrder::BigEndian);
    
    /**
     * @brief 从字节数组提取校验和
     * @param data 字节数组
     * @param offset 偏移
     * @param length 长度
     * @param byteOrder 字节序
     * @return 校验值
     */
    static uint64_t fromBytes(const QByteArray& data, int offset, int length, 
                              ByteOrder byteOrder = ByteOrder::BigEndian);
};

/**
 * @brief CRC查找表生成器
 */
class CrcTable
{
public:
    /**
     * @brief 获取CRC8查找表
     * @param poly 多项式
     * @return 查找表
     */
    static const uint8_t* getCrc8Table(uint8_t poly);
    
    /**
     * @brief 获取CRC16查找表
     * @param poly 多项式
     * @return 查找表
     */
    static const uint16_t* getCrc16Table(uint16_t poly);

private:
    static uint8_t generateCrc8Byte(uint8_t byte, uint8_t poly);
    static uint16_t generateCrc16Byte(uint16_t byte, uint16_t poly);
};

} // namespace DeviceStudio

/**
 * @file scriptparser.h
 * @brief 脚本命令解析器
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include "scriptcommands.h"
#include <QString>
#include <QStringList>
#include <QRegularExpression>

namespace DeviceStudio {

/**
 * @brief 脚本命令解析器
 * 
 * 解析简单的脚本命令语言，支持：
 * - 设备连接命令
 * - 数据发送命令
 * - 控制结构
 * - 变量操作
 */
class ScriptParser
{
public:
    ScriptParser();
    ~ScriptParser() = default;
    
    /**
     * @brief 解析脚本文本
     * @param script 脚本内容
     * @return 解析结果
     */
    ParseResult parse(const QString& script);
    
    /**
     * @brief 解析脚本文件
     * @param filePath 脚本文件路径
     * @return 解析结果
     */
    ParseResult parseFile(const QString& filePath);
    
    /**
     * @brief 验证脚本语法
     */
    bool validate(const QString& script, QString& errorMessage);
    
    /**
     * @brief 获取命令列表
     */
    QVector<ScriptCommand> getCommands() const { return commands_; }
    
    /**
     * @brief 获取标签位置映射
     */
    QMap<QString, int> getLabels() const { return labels_; }

private:
    /**
     * @brief 解析单行命令
     */
    ScriptCommand parseLine(const QString& line, int lineNumber);
    
    /**
     * @brief 提取命令类型
     */
    CommandType extractCommandType(const QString& command);
    
    /**
     * @brief 解析参数
     */
    QVector<QString> parseArguments(const QString& args);
    
    /**
     * @brief 解析条件表达式
     */
    QString parseCondition(const QString& args);
    
    /**
     * @brief 解析循环参数
     */
    bool parseLoopParams(const QString& args, QString& variable, int& start, int& end);
    
    /**
     * @brief 移除注释
     */
    QString removeComment(const QString& line);
    
    /**
     * @brief 验证控制结构配对
     */
    bool validateControlStructures(QString& errorMessage);
    
    /**
     * @brief 解析HEX字符串
     */
    QByteArray parseHexString(const QString& hexStr);
    
    QVector<ScriptCommand> commands_;
    QMap<QString, int> labels_;
    
    // 正则表达式
    QRegularExpression ifRegex_;
    QRegularExpression forRegex_;
    QRegularExpression setRegex_;
    QRegularExpression extractRegex_;
};

} // namespace DeviceStudio

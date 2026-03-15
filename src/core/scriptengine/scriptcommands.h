/**
 * @file scriptcommands.h
 * @brief 脚本命令定义
 * @author DeviceStudio Team
 * @date 2026-03-15
 * 
 * 脚本命令系统支持：
 * - CONNECT/DISCONNECT - 设备连接控制
 * - SEND/SENDHEX - 数据发送
 * - WAIT/DELAY - 延时等待
 * - EXPECT/EXPECTHEX - 期望接收
 * - IF/ELSE/ENDIF - 条件判断
 * - FOR/ENDFOR - 循环
 * - SET - 变量赋值
 * - LOG - 日志输出
 * - EXTRACT - 数据提取
 * - PLOT - 图表绑定
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QMap>
#include <QVector>

namespace DeviceStudio {

/**
 * @brief 命令类型
 */
enum class CommandType {
    // 连接控制
    Connect,
    Disconnect,
    
    // 数据发送
    Send,
    SendHex,
    
    // 等待和期望
    Wait,
    Delay,
    Expect,
    ExpectHex,
    
    // 控制结构
    If,
    Else,
    EndIf,
    For,
    EndFor,
    Break,
    Continue,
    
    // 变量和日志
    Set,
    Log,
    
    // 高级命令
    Extract,
    Plot,
    
    // 特殊
    Label,      // 标签（用于跳转）
    Goto,       // 跳转
    Comment,    // 注释
    Empty       // 空行
};

/**
 * @brief 命令结构
 */
struct ScriptCommand {
    CommandType type = CommandType::Empty;
    QString rawLine;                    // 原始行
    int lineNumber = 0;                 // 行号
    QString commandName;                // 命令名称
    QVector<QString> arguments;         // 参数列表
    
    // 控制结构相关
    QString condition;                  // 条件表达式（IF）
    QString loopVariable;               // 循环变量（FOR）
    int loopStart = 0;                  // 循环起始值
    int loopEnd = 0;                    // 循环结束值
    int jumpTarget = -1;                // 跳转目标行号
    
    // 变量相关
    QString variableName;               // 变量名
    QVariant variableValue;             // 变量值
};

/**
 * @brief 脚本执行上下文
 */
struct ScriptContext {
    // 变量表
    QMap<QString, QVariant> variables;
    
    // 当前连接的设备名
    QString currentDevice;
    
    // 最后接收的数据
    QByteArray lastReceivedData;
    
    // 最后期望匹配结果
    bool lastExpectMatch = false;
    
    // 循环栈
    struct LoopInfo {
        int startLine;
        int endLine;
        QString variable;
        int current;
        int end;
    };
    QVector<LoopInfo> loopStack;
    
    // 条件栈（用于IF/ELSE嵌套）
    struct ConditionInfo {
        bool condition;
        bool elseBranch;
        bool executed;
    };
    QVector<ConditionInfo> conditionStack;
    
    // 提取的数据
    QMap<QString, QVariant> extractedData;
    
    // 执行统计
    int executedLines = 0;
    int currentLine = 0;
    bool stopRequested = false;
};

/**
 * @brief 命令解析结果
 */
struct ParseResult {
    bool success = false;
    QString errorMessage;
    int errorLine = 0;
    QVector<ScriptCommand> commands;
};

/**
 * @brief 执行结果
 */
struct ExecutionResult {
    bool success = false;
    QString message;
    int executedLines = 0;
    QString errorDetail;
};

} // namespace DeviceStudio

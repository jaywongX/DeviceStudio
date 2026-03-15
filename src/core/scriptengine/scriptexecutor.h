/**
 * @file scriptexecutor.h
 * @brief 脚本命令执行器
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#pragma once

#include "scriptcommands.h"
#include "scriptparser.h"
#include <QObject>
#include <QTimer>
#include <functional>

namespace DeviceStudio {

class DeviceManager;
class DataStore;

/**
 * @brief 脚本执行器
 * 
 * 执行解析后的脚本命令，支持：
 * - 设备连接/断开
 * - 数据发送/接收
 * - 条件判断和循环
 * - 变量管理
 */
class ScriptExecutor : public QObject
{
    Q_OBJECT

public:
    explicit ScriptExecutor(QObject* parent = nullptr);
    ~ScriptExecutor() override;
    
    // ========== 配置 ==========
    
    /**
     * @brief 设置设备管理器
     */
    void setDeviceManager(DeviceManager* manager);
    
    /**
     * @brief 设置数据存储
     */
    void setDataStore(DataStore* store);
    
    /**
     * @brief 设置图表回调（用于PLOT命令）
     */
    void setPlotCallback(std::function<void(const QString&, double)> callback);
    
    // ========== 执行控制 ==========
    
    /**
     * @brief 加载并验证脚本
     */
    bool loadScript(const QString& script);
    
    /**
     * @brief 加载脚本文件
     */
    bool loadScriptFile(const QString& filePath);
    
    /**
     * @brief 开始执行
     */
    void start();
    
    /**
     * @brief 暂停执行
     */
    void pause();
    
    /**
     * @brief 继续执行
     */
    void resume();
    
    /**
     * @brief 停止执行
     */
    void stop();
    
    /**
     * @brief 是否正在执行
     */
    bool isRunning() const { return isRunning_; }
    
    /**
     * @brief 是否已暂停
     */
    bool isPaused() const { return isPaused_; }
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取当前行号
     */
    int currentLine() const { return context_.currentLine; }
    
    /**
     * @brief 获取执行上下文
     */
    const ScriptContext& context() const { return context_; }
    
    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return lastError_; }

signals:
    /**
     * @brief 执行开始信号
     */
    void executionStarted();
    
    /**
     * @brief 执行完成信号
     */
    void executionFinished(bool success, const QString& message);
    
    /**
     * @brief 行执行信号
     */
    void lineExecuted(int lineNumber, const QString& command);
    
    /**
     * @brief 日志输出信号
     */
    void logOutput(const QString& message);
    
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);
    
    /**
     * @brief 数据发送信号
     */
    void dataSent(const QByteArray& data);
    
    /**
     * @brief 数据接收信号
     */
    void dataReceived(const QByteArray& data);
    
    /**
     * @brief 设备连接信号
     */
    void deviceConnected(const QString& deviceName);
    
    /**
     * @brief 设备断开信号
     */
    void deviceDisconnected(const QString& deviceName);

private slots:
    /**
     * @brief 执行下一行命令
     */
    void executeNextLine();
    
    /**
     * @brief 数据接收处理
     */
    void onDataReceived(const QByteArray& data);

private:
    // ========== 命令执行 ==========
    
    ExecutionResult executeCommand(const ScriptCommand& cmd);
    ExecutionResult executeConnect(const ScriptCommand& cmd);
    ExecutionResult executeDisconnect(const ScriptCommand& cmd);
    ExecutionResult executeSend(const ScriptCommand& cmd);
    ExecutionResult executeSendHex(const ScriptCommand& cmd);
    ExecutionResult executeWait(const ScriptCommand& cmd);
    ExecutionResult executeExpect(const ScriptCommand& cmd);
    ExecutionResult executeExpectHex(const ScriptCommand& cmd);
    ExecutionResult executeIf(const ScriptCommand& cmd);
    ExecutionResult executeElse(const ScriptCommand& cmd);
    ExecutionResult executeEndIf(const ScriptCommand& cmd);
    ExecutionResult executeFor(const ScriptCommand& cmd);
    ExecutionResult executeEndFor(const ScriptCommand& cmd);
    ExecutionResult executeSet(const ScriptCommand& cmd);
    ExecutionResult executeLog(const ScriptCommand& cmd);
    ExecutionResult executeExtract(const ScriptCommand& cmd);
    ExecutionResult executePlot(const ScriptCommand& cmd);
    
    // ========== 辅助函数 ==========
    
    /**
     * @brief 替换变量
     */
    QString replaceVariables(const QString& text);
    
    /**
     * @brief 计算条件表达式
     */
    bool evaluateCondition(const QString& condition);
    
    /**
     * @brief 获取变量值
     */
    QVariant getVariable(const QString& name);
    
    /**
     * @brief 设置变量值
     */
    void setVariable(const QString& name, const QVariant& value);
    
    /**
     * @brief 检查是否应该跳过执行（条件分支）
     */
    bool shouldSkipExecution();
    
    /**
     * @brief 等待数据接收
     */
    bool waitForData(int timeoutMs);
    
    // 成员变量
    ScriptParser parser_;
    QVector<ScriptCommand> commands_;
    ScriptContext context_;
    
    DeviceManager* deviceManager_ = nullptr;
    DataStore* dataStore_ = nullptr;
    std::function<void(const QString&, double)> plotCallback_;
    
    bool isRunning_ = false;
    bool isPaused_ = false;
    QString lastError_;
    
    QTimer* executionTimer_ = nullptr;
    QTimer* waitTimer_ = nullptr;
    QByteArray pendingReceiveData_;
    bool expectMatch_ = false;
};

} // namespace DeviceStudio

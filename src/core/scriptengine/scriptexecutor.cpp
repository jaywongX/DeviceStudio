/**
 * @file scriptexecutor.cpp
 * @brief 脚本命令执行器实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "scriptexecutor.h"
#include "core/devicemanager/devicemanager.h"
#include "core/datacenter/datastore.h"
#include <QFile>
#include <QRegularExpression>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

namespace DeviceStudio {

ScriptExecutor::ScriptExecutor(QObject* parent)
    : QObject(parent)
{
    executionTimer_ = new QTimer(this);
    waitTimer_ = new QTimer(this);
    
    connect(waitTimer_, &QTimer::timeout, this, [this]() {
        waitTimer_->stop();
    });
}

ScriptExecutor::~ScriptExecutor()
{
    stop();
}

void ScriptExecutor::setDeviceManager(DeviceManager* manager)
{
    deviceManager_ = manager;
}

void ScriptExecutor::setDataStore(DataStore* store)
{
    dataStore_ = store;
}

void ScriptExecutor::setPlotCallback(std::function<void(const QString&, double)> callback)
{
    plotCallback_ = callback;
}

// ========== 执行控制 ==========

ExecutionResult ScriptExecutor::load(const QString& script)
{
    ExecutionResult result;
    
    ParseResult parseResult = parser_.parse(script);
    if (!parseResult.success) {
        result.success = false;
        result.message = parseResult.errorMessage;
        result.errorDetail = QString("解析错误: 第 %1 行").arg(parseResult.errorLine);
        return result;
    }
    
    commands_ = parseResult.commands;
    result.success = true;
    result.message = QString("脚本加载成功，共 %1 行命令").arg(commands_.size());
    return result;
}

ExecutionResult ScriptExecutor::loadFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ExecutionResult result;
        result.success = false;
        result.message = QString("无法打开文件: %1").arg(filePath);
        return result;
    }
    
    QString script = QString::fromUtf8(file.readAll());
    file.close();
    
    return load(script);
}

ExecutionResult ScriptExecutor::execute()
{
    ExecutionResult result;
    
    if (commands_.isEmpty()) {
        result.success = false;
        result.message = "没有可执行的命令";
        return result;
    }
    
    // 重置上下文
    context_ = ScriptContext();
    isRunning_ = true;
    isPaused_ = false;
    context_.stopRequested = false;
    
    emit executionStarted();
    
    for (int i = 0; i < commands_.size(); ++i) {
        if (context_.stopRequested) {
            result.success = false;
            result.message = "执行被用户终止";
            break;
        }
        
        // 检查暂停
        while (isPaused_ && isRunning_) {
            QThread::msleep(10);
            QCoreApplication::processEvents();
        }
        
        if (!isRunning_) break;
        
        context_.currentLine = i;
        const ScriptCommand& cmd = commands_[i];
        
        emit lineExecuting(cmd.lineNumber, cmd.rawLine);
        
        ExecutionResult cmdResult = executeCommand(cmd, i);
        context_.executedLines++;
        
        if (!cmdResult.success) {
            result.success = false;
            result.message = cmdResult.message;
            result.errorDetail = QString("第 %1 行: %2").arg(cmd.lineNumber).arg(cmd.rawLine);
            emit errorOccurred(result.message, cmd.lineNumber);
            break;
        }
        
        // 处理跳转
        if (cmdResult.executedLines > 0) {
            // 循环跳转
            i = cmdResult.executedLines - 1; // -1 因为循环会 +1
        }
        
        // 让UI有机会更新
        QCoreApplication::processEvents();
    }
    
    if (result.success || result.message.isEmpty()) {
        result.success = true;
        result.message = "脚本执行完成";
    }
    
    result.executedLines = context_.executedLines;
    isRunning_ = false;
    
    emit executionFinished(result.success, result.message);
    return result;
}

void ScriptExecutor::executeAsync()
{
    // 在后台线程执行
    QtConcurrent::run([this]() {
        execute();
    });
}

void ScriptExecutor::stop()
{
    isRunning_ = false;
    isPaused_ = false;
    context_.stopRequested = true;
    waitTimer_->stop();
    emit executionFinished(false, "执行已停止");
}

void ScriptExecutor::pause()
{
    isPaused_ = true;
    emit executionPaused();
}

void ScriptExecutor::resume()
{
    isPaused_ = false;
    emit executionResumed();
}

bool ScriptExecutor::isRunning() const
{
    return isRunning_;
}

bool ScriptExecutor::isPaused() const
{
    return isPaused_;
}

ScriptContext ScriptExecutor::getContext() const
{
    return context_;
}

// ========== 命令执行 ==========

ExecutionResult ScriptExecutor::executeCommand(const ScriptCommand& cmd, int& currentIndex)
{
    ExecutionResult result;
    result.success = true;
    
    // 检查是否应该跳过执行（条件分支）
    bool skip = shouldSkipExecution();
    
    switch (cmd.type) {
        case CommandType::Connect:
            if (!skip) result = executeConnect(cmd);
            break;
            
        case CommandType::Disconnect:
            if (!skip) result = executeDisconnect(cmd);
            break;
            
        case CommandType::Send:
        case CommandType::SendHex:
            if (!skip) result = executeSend(cmd);
            break;
            
        case CommandType::Wait:
        case CommandType::Delay:
            if (!skip) result = executeWait(cmd);
            break;
            
        case CommandType::Expect:
        case CommandType::ExpectHex:
            if (!skip) result = executeExpect(cmd);
            break;
            
        case CommandType::If:
            result = executeIf(cmd);
            break;
            
        case CommandType::Else:
            // 在 executeIf 中处理
            break;
            
        case CommandType::EndIf:
            // 弹出条件栈
            if (!context_.conditionStack.isEmpty()) {
                context_.conditionStack.pop_back();
            }
            break;
            
        case CommandType::For:
            result = executeFor(cmd, currentIndex);
            break;
            
        case CommandType::EndFor:
            result = executeEndFor(cmd, currentIndex);
            break;
            
        case CommandType::Break:
            if (!skip && !context_.loopStack.isEmpty()) {
                // 跳转到ENDFOR之后
                currentIndex = context_.loopStack.last().endLine;
                context_.loopStack.pop_back();
            }
            break;
            
        case CommandType::Continue:
            if (!skip && !context_.loopStack.isEmpty()) {
                // 跳转回FOR
                currentIndex = context_.loopStack.last().startLine - 1;
            }
            break;
            
        case CommandType::Set:
            if (!skip) result = executeSet(cmd);
            break;
            
        case CommandType::Log:
            if (!skip) result = executeLog(cmd);
            break;
            
        case CommandType::Extract:
            if (!skip) result = executeExtract(cmd);
            break;
            
        case CommandType::Plot:
            if (!skip) result = executePlot(cmd);
            break;
            
        case CommandType::Comment:
        case CommandType::Empty:
        case CommandType::Label:
            // 忽略
            break;
            
        default:
            break;
    }
    
    return result;
}

// ========== 具体命令执行 ==========

ExecutionResult ScriptExecutor::executeConnect(const ScriptCommand& cmd)
{
    ExecutionResult result;
    
    if (cmd.arguments.size() < 2) {
        result.success = false;
        result.message = "CONNECT 命令参数不足";
        return result;
    }
    
    QString type = cmd.arguments[0].toUpper();
    
    if (type == "COM" || type == "SERIAL") {
        // 串口连接: CONNECT COM3 115200
        QString port = cmd.arguments[1];
        int baudrate = cmd.arguments.size() > 2 ? cmd.arguments[2].toInt() : 115200;
        
        if (deviceManager_) {
            // TODO: 调用设备管理器连接
            context_.currentDevice = port;
            emit logOutput(QString("连接串口: %1, 波特率: %2").arg(port).arg(baudrate));
        }
    } else if (type == "TCP") {
        // TCP连接: CONNECT TCP 192.168.1.100 5000
        if (cmd.arguments.size() < 3) {
            result.success = false;
            result.message = "TCP连接参数不足";
            return result;
        }
        QString host = cmd.arguments[1];
        int port = cmd.arguments[2].toInt();
        
        if (deviceManager_) {
            // TODO: 调用设备管理器连接
            context_.currentDevice = QString("%1:%2").arg(host).arg(port);
            emit logOutput(QString("连接TCP: %1:%2").arg(host).arg(port));
        }
    } else {
        result.success = false;
        result.message = QString("未知的连接类型: %1").arg(type);
        return result;
    }
    
    result.success = true;
    return result;
}

ExecutionResult ScriptExecutor::executeDisconnect(const ScriptCommand& cmd)
{
    ExecutionResult result;
    
    if (deviceManager_ && !context_.currentDevice.isEmpty()) {
        // TODO: 调用设备管理器断开
        emit logOutput(QString("断开设备: %1").arg(context_.currentDevice));
    }
    
    context_.currentDevice.clear();
    result.success = true;
    return result;
}

ExecutionResult ScriptExecutor::executeSend(const ScriptCommand& cmd)
{
    ExecutionResult result;
    
    if (cmd.arguments.isEmpty()) {
        result.success = false;
        result.message = "SEND 命令缺少数据参数";
        return result;
    }
    
    QString dataStr = replaceVariables(cmd.arguments[0]);
    QByteArray data;
    
    if (cmd.type == CommandType::SendHex) {
        // HEX格式
        data = QByteArray::fromHex(dataStr.toLatin1());
    } else {
        // ASCII格式
        data = dataStr.toUtf8();
    }
    
    if (deviceManager_ && !context_.currentDevice.isEmpty()) {
        emit sendDataRequested(context_.currentDevice, data);
        emit logOutput(QString("发送: %1").arg(dataStr));
    }
    
    result.success = true;
    return result;
}

ExecutionResult ScriptExecutor::executeWait(const ScriptCommand& cmd)
{
    ExecutionResult result;
    
    if (cmd.arguments.isEmpty()) {
        result.success = false;
        result.message = "WAIT 命令缺少时间参数";
        return result;
    }
    
    int ms = replaceVariables(cmd.arguments[0]).toInt();
    ms = qMax(1, ms);
    
    emit logOutput(QString("等待: %1 ms").arg(ms));
    
    // 使用事件循环等待
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
    
    result.success = true;
    return result;
}

ExecutionResult ScriptExecutor::executeExpect(const ScriptCommand& cmd)
{
    ExecutionResult result;
    
    if (cmd.arguments.isEmpty()) {
        result.success = false;
        result.message = "EXPECT 命令缺少期望数据";
        return result;
    }
    
    QString expected = replaceVariables(cmd.arguments[0]);
    
    // 等待接收数据
    int timeout = cmd.arguments.size() > 1 ? cmd.arguments[1].toInt() : 5000;
    
    if (waitForData(timeout)) {
        QString received;
        if (cmd.type == CommandType::ExpectHex) {
            received = context_.lastReceivedData.toHex(' ').toUpper();
        } else {
            received = QString::fromUtf8(context_.lastReceivedData);
        }
        
        bool match = received.contains(expected);
        context_.lastExpectMatch = match;
        
        if (match) {
            emit logOutput(QString("期望匹配成功: %1").arg(expected));
            result.success = true;
        } else {
            emit logOutput(QString("期望匹配失败: 期望'%1', 实际'%2'").arg(expected).arg(received));
            result.success = false;
            result.message = "期望数据不匹配";
        }
    } else {
        result.success = false;
        result.message = QString("等待接收超时 (%1ms)").arg(timeout);
    }
    
    return result;
}

ExecutionResult ScriptExecutor::executeIf(const ScriptCommand& cmd)
{
    ExecutionResult result;
    result.success = true;
    
    bool conditionResult = evaluateCondition(cmd.condition);
    
    ScriptContext::ConditionInfo info;
    info.condition = conditionResult;
    info.elseBranch = false;
    info.executed = conditionResult;
    
    context_.conditionStack.push_back(info);
    
    return result;
}

ExecutionResult ScriptExecutor::executeFor(const ScriptCommand& cmd, int& currentIndex)
{
    ExecutionResult result;
    result.success = true;
    
    ScriptContext::LoopInfo loop;
    loop.startLine = currentIndex;
    loop.variable = cmd.loopVariable;
    loop.current = cmd.loopStart;
    loop.end = cmd.loopEnd;
    
    // 查找对应的ENDFOR
    int depth = 1;
    for (int i = currentIndex + 1; i < commands_.size(); ++i) {
        if (commands_[i].type == CommandType::For) depth++;
        else if (commands_[i].type == CommandType::EndFor) {
            depth--;
            if (depth == 0) {
                loop.endLine = i;
                break;
            }
        }
    }
    
    context_.loopStack.push_back(loop);
    setVariable(loop.variable, loop.current);
    
    emit logOutput(QString("循环开始: %1 = %2 TO %3").arg(loop.variable).arg(loop.current).arg(loop.end));
    
    return result;
}

ExecutionResult ScriptExecutor::executeEndFor(const ScriptCommand& cmd, int& currentIndex)
{
    ExecutionResult result;
    result.success = true;
    
    if (context_.loopStack.isEmpty()) {
        return result;
    }
    
    ScriptContext::LoopInfo& loop = context_.loopStack.last();
    loop.current++;
    
    if (loop.current <= loop.end) {
        // 继续循环
        setVariable(loop.variable, loop.current);
        currentIndex = loop.startLine;  // 跳回FOR
        result.executedLines = loop.startLine + 1;  // 设置跳转目标
    } else {
        // 循环结束
        emit logOutput(QString("循环结束: %1").arg(loop.variable));
        context_.loopStack.pop_back();
    }
    
    return result;
}

ExecutionResult ScriptExecutor::executeSet(const ScriptCommand& cmd)
{
    ExecutionResult result;
    result.success = true;
    
    QString value = replaceVariables(cmd.variableValue.toString());
    setVariable(cmd.variableName, value);
    
    emit logOutput(QString("设置变量: %1 = %2").arg(cmd.variableName).arg(value));
    
    return result;
}

ExecutionResult ScriptExecutor::executeLog(const ScriptCommand& cmd)
{
    ExecutionResult result;
    result.success = true;
    
    QString message = replaceVariables(cmd.arguments.isEmpty() ? QString() : cmd.arguments[0]);
    emit logOutput(message);
    
    return result;
}

ExecutionResult ScriptExecutor::executeExtract(const ScriptCommand& cmd)
{
    ExecutionResult result;
    result.success = true;
    
    if (cmd.arguments.size() < 2) {
        result.success = false;
        result.message = "EXTRACT 命令参数不足";
        return result;
    }
    
    QString field = cmd.arguments[0];
    QString source = cmd.arguments[1];
    
    // 从接收数据中提取
    QString data = QString::fromUtf8(context_.lastReceivedData);
    
    // 简单提取：查找 field=value 或 field: value 格式
    QRegularExpression regex(QString(R"(%1\s*[=:]\s*(\S+))").arg(field));
    QRegularExpressionMatch match = regex.match(data);
    
    if (match.hasMatch()) {
        QString value = match.captured(1);
        setVariable("$" + field, value);
        context_.extractedData[field] = value;
        emit logOutput(QString("提取数据: %1 = %2").arg(field).arg(value));
    } else {
        emit logOutput(QString("未能提取字段: %1").arg(field));
    }
    
    return result;
}

ExecutionResult ScriptExecutor::executePlot(const ScriptCommand& cmd)
{
    ExecutionResult result;
    result.success = true;
    
    if (cmd.arguments.size() < 2) {
        result.success = false;
        result.message = "PLOT 命令参数不足";
        return result;
    }
    
    QString channel = cmd.arguments[0];
    double value = replaceVariables(cmd.arguments[1]).toDouble();
    
    if (plotCallback_) {
        plotCallback_(channel, value);
    }
    
    emit logOutput(QString("图表绑定: %1 = %2").arg(channel).arg(value));
    
    return result;
}

// ========== 辅助函数 ==========

QString ScriptExecutor::replaceVariables(const QString& text)
{
    QString result = text;
    
    // 替换变量 $varname
    QRegularExpression varRegex(R"(\$(\w+))");
    QRegularExpressionMatchIterator it = varRegex.globalMatch(text);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = "$" + match.captured(1);
        QVariant value = getVariable(varName);
        
        if (value.isValid()) {
            result.replace(varName, value.toString());
        }
    }
    
    // 特殊变量
    result.replace("$RESPONSE", QString::fromUtf8(context_.lastReceivedData));
    result.replace("$TIME", QDateTime::currentDateTime().toString("hh:mm:ss"));
    result.replace("$DATE", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    
    return result;
}

bool ScriptExecutor::evaluateCondition(const QString& condition)
{
    QString cond = replaceVariables(condition).trimmed();
    
    // 解析比较操作符
    static QRegularExpression compareRegex(R"((.+?)\s*(==|!=|>=|<=|>|<)\s*(.+))");
    QRegularExpressionMatch match = compareRegex.match(cond);
    
    if (match.hasMatch()) {
        QString left = match.captured(1).trimmed();
        QString op = match.captured(2);
        QString right = match.captured(3).trimmed();
        
        // 数值比较
        bool leftOk, rightOk;
        double leftNum = left.toDouble(&leftOk);
        double rightNum = right.toDouble(&rightOk);
        
        if (leftOk && rightOk) {
            if (op == "==") return qFuzzyCompare(leftNum, rightNum);
            if (op == "!=") return !qFuzzyCompare(leftNum, rightNum);
            if (op == ">=") return leftNum >= rightNum;
            if (op == "<=") return leftNum <= rightNum;
            if (op == ">") return leftNum > rightNum;
            if (op == "<") return leftNum < rightNum;
        } else {
            // 字符串比较
            if (op == "==") return left == right;
            if (op == "!=") return left != right;
            if (op == ">=") return left >= right;
            if (op == "<=") return left <= right;
            if (op == ">") return left > right;
            if (op == "<") return left < right;
        }
    }
    
    // 简单布尔检查
    if (cond.toLower() == "true" || cond == "1") return true;
    if (cond.toLower() == "false" || cond == "0") return false;
    
    return !cond.isEmpty();
}

QVariant ScriptExecutor::getVariable(const QString& name)
{
    QString varName = name.startsWith('$') ? name : "$" + name;
    return context_.variables.value(varName);
}

void ScriptExecutor::setVariable(const QString& name, const QVariant& value)
{
    QString varName = name.startsWith('$') ? name : "$" + name;
    context_.variables[varName] = value;
}

bool ScriptExecutor::shouldSkipExecution()
{
    if (context_.conditionStack.isEmpty()) {
        return false;
    }
    
    const auto& info = context_.conditionStack.last();
    return !info.executed;
}

bool ScriptExecutor::waitForData(int timeoutMs)
{
    // 简化实现：假设数据已经接收
    // 实际实现需要与通信模块集成
    return !context_.lastReceivedData.isEmpty();
}

void ScriptExecutor::onDataReceived(const QString& deviceName, const QByteArray& data)
{
    if (deviceName == context_.currentDevice) {
        context_.lastReceivedData = data;
        emit logOutput(QString("接收: %1").arg(QString::fromUtf8(data)));
    }
}

void ScriptExecutor::onDeviceConnected(const QString& deviceName)
{
    emit logOutput(QString("设备已连接: %1").arg(deviceName));
}

void ScriptExecutor::onDeviceDisconnected(const QString& deviceName)
{
    emit logOutput(QString("设备已断开: %1").arg(deviceName));
}

} // namespace DeviceStudio

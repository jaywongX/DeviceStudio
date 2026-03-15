/**
 * @file scriptparser.cpp
 * @brief 脚本命令解析器实现
 * @author DeviceStudio Team
 * @date 2026-03-15
 */

#include "scriptparser.h"
#include <QFile>
#include <QDebug>

namespace DeviceStudio {

ScriptParser::ScriptParser()
{
    // 初始化正则表达式
    // IF $var == value 或 IF $var != value
    ifRegex_.setPattern(R"(IF\s+(\$\w+)\s*(==|!=|>|<|>=|<=)\s*(.+))");
    // FOR $i = 1 TO 10
    forRegex_.setPattern(R"(FOR\s+(\$\w+)\s*=\s*(\d+)\s+TO\s+(\d+))");
    // SET $var = value
    setRegex_.setPattern(R"(SET\s+(\$\w+)\s*=\s*(.+))");
    // EXTRACT field FROM $RESPONSE
    extractRegex_.setPattern(R"(EXTRACT\s+(\w+)\s+FROM\s+(\$\w+))");
}

ParseResult ScriptParser::parse(const QString& script)
{
    ParseResult result;
    commands_.clear();
    labels_.clear();
    
    QStringList lines = script.split('\n');
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        ScriptCommand cmd = parseLine(line, i + 1);
        commands_.append(cmd);
        
        // 记录标签位置
        if (cmd.type == CommandType::Label) {
            if (!cmd.arguments.isEmpty()) {
                labels_[cmd.arguments[0]] = i;
            }
        }
    }
    
    // 验证控制结构
    QString errorMsg;
    if (!validateControlStructures(errorMsg)) {
        result.errorMessage = errorMsg;
        return result;
    }
    
    result.success = true;
    result.commands = commands_;
    return result;
}

ParseResult ScriptParser::parseFile(const QString& filePath)
{
    ParseResult result;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QString("无法打开文件: %1").arg(filePath);
        return result;
    }
    
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    
    return parse(content);
}

bool ScriptParser::validate(const QString& script, QString& errorMessage)
{
    ParseResult result = parse(script);
    if (!result.success) {
        errorMessage = result.errorMessage;
        return false;
    }
    return true;
}

ScriptCommand ScriptParser::parseLine(const QString& line, int lineNumber)
{
    ScriptCommand cmd;
    cmd.rawLine = line;
    cmd.lineNumber = lineNumber;
    
    // 移除注释
    QString cleanLine = removeComment(line).trimmed();
    
    // 空行
    if (cleanLine.isEmpty()) {
        cmd.type = CommandType::Empty;
        return cmd;
    }
    
    // 注释行
    if (cleanLine.startsWith('#')) {
        cmd.type = CommandType::Comment;
        return cmd;
    }
    
    // 标签 (label:)
    if (cleanLine.endsWith(':') && !cleanLine.contains(' ')) {
        cmd.type = CommandType::Label;
        cmd.arguments.append(cleanLine.left(cleanLine.length() - 1));
        return cmd;
    }
    
    // 提取命令和参数
    int spacePos = cleanLine.indexOf(' ');
    QString commandPart = (spacePos > 0) ? cleanLine.left(spacePos).toUpper() : cleanLine.toUpper();
    QString argsPart = (spacePos > 0) ? cleanLine.mid(spacePos + 1).trimmed() : "";
    
    cmd.commandName = commandPart;
    cmd.type = extractCommandType(commandPart);
    
    // 根据命令类型解析参数
    switch (cmd.type) {
        case CommandType::Connect: {
            // CONNECT COM3 115200 或 CONNECT TCP 192.168.1.100 5000
            cmd.arguments = parseArguments(argsPart);
            break;
        }
        
        case CommandType::Send:
        case CommandType::SendHex: {
            // SEND/SENDHEX data
            cmd.arguments.append(argsPart);
            break;
        }
        
        case CommandType::Wait:
        case CommandType::Delay: {
            // WAIT/DELAY milliseconds
            cmd.arguments.append(argsPart);
            break;
        }
        
        case CommandType::Expect:
        case CommandType::ExpectHex: {
            // EXPECT/EXPECTHEX pattern
            cmd.arguments.append(argsPart);
            break;
        }
        
        case CommandType::If: {
            // IF condition
            cmd.condition = argsPart;
            break;
        }
        
        case CommandType::For: {
            // FOR $i = 1 TO 10
            parseLoopParams(argsPart, cmd.loopVariable, cmd.loopStart, cmd.loopEnd);
            break;
        }
        
        case CommandType::Set: {
            // SET $var = value
            QRegularExpressionMatch match = setRegex_.match(argsPart);
            if (match.hasMatch()) {
                cmd.variableName = match.captured(1);
                cmd.variableValue = match.captured(2).trimmed();
            }
            break;
        }
        
        case CommandType::Log: {
            // LOG message
            cmd.arguments.append(argsPart);
            break;
        }
        
        case CommandType::Extract: {
            // EXTRACT field FROM $RESPONSE
            QRegularExpressionMatch match = extractRegex_.match(argsPart);
            if (match.hasMatch()) {
                cmd.arguments.append(match.captured(1));
                cmd.arguments.append(match.captured(2));
            }
            break;
        }
        
        case CommandType::Plot: {
            // PLOT channel value
            cmd.arguments = parseArguments(argsPart);
            break;
        }
        
        case CommandType::Goto: {
            // GOTO label
            cmd.arguments.append(argsPart);
            break;
        }
        
        default:
            cmd.arguments = parseArguments(argsPart);
            break;
    }
    
    return cmd;
}

CommandType ScriptParser::extractCommandType(const QString& command)
{
    QString cmd = command.toUpper();
    
    if (cmd == "CONNECT") return CommandType::Connect;
    if (cmd == "DISCONNECT") return CommandType::Disconnect;
    if (cmd == "SEND") return CommandType::Send;
    if (cmd == "SENDHEX") return CommandType::SendHex;
    if (cmd == "WAIT") return CommandType::Wait;
    if (cmd == "DELAY") return CommandType::Delay;
    if (cmd == "EXPECT") return CommandType::Expect;
    if (cmd == "EXPECTHEX") return CommandType::ExpectHex;
    if (cmd == "IF") return CommandType::If;
    if (cmd == "ELSE") return CommandType::Else;
    if (cmd == "ENDIF") return CommandType::EndIf;
    if (cmd == "FOR") return CommandType::For;
    if (cmd == "ENDFOR") return CommandType::EndFor;
    if (cmd == "BREAK") return CommandType::Break;
    if (cmd == "CONTINUE") return CommandType::Continue;
    if (cmd == "SET") return CommandType::Set;
    if (cmd == "LOG") return CommandType::Log;
    if (cmd == "EXTRACT") return CommandType::Extract;
    if (cmd == "PLOT") return CommandType::Plot;
    if (cmd == "GOTO") return CommandType::Goto;
    
    return CommandType::Empty;
}

QVector<QString> ScriptParser::parseArguments(const QString& args)
{
    QVector<QString> result;
    QString current;
    bool inQuote = false;
    
    for (int i = 0; i < args.length(); ++i) {
        QChar ch = args[i];
        
        if (ch == '"') {
            inQuote = !inQuote;
        } else if (ch == ' ' && !inQuote) {
            if (!current.isEmpty()) {
                result.append(current);
                current.clear();
            }
        } else {
            current.append(ch);
        }
    }
    
    if (!current.isEmpty()) {
        result.append(current);
    }
    
    return result;
}

QString ScriptParser::parseCondition(const QString& args)
{
    return args.trimmed();
}

bool ScriptParser::parseLoopParams(const QString& args, QString& variable, int& start, int& end)
{
    QRegularExpressionMatch match = forRegex_.match(args);
    if (match.hasMatch()) {
        variable = match.captured(1);
        start = match.captured(2).toInt();
        end = match.captured(3).toInt();
        return true;
    }
    return false;
}

QString ScriptParser::removeComment(const QString& line)
{
    // 移除行尾注释（#后面不是在字符串内）
    int commentPos = -1;
    bool inQuote = false;
    
    for (int i = 0; i < line.length(); ++i) {
        if (line[i] == '"') {
            inQuote = !inQuote;
        } else if (!inQuote && line[i] == '#') {
            commentPos = i;
            break;
        }
    }
    
    return (commentPos >= 0) ? line.left(commentPos) : line;
}

bool ScriptParser::validateControlStructures(QString& errorMessage)
{
    int ifDepth = 0;
    int forDepth = 0;
    int lastIfLine = -1;
    int lastForLine = -1;
    
    for (int i = 0; i < commands_.size(); ++i) {
        const ScriptCommand& cmd = commands_[i];
        
        switch (cmd.type) {
            case CommandType::If:
                ifDepth++;
                lastIfLine = cmd.lineNumber;
                break;
                
            case CommandType::Else:
                if (ifDepth == 0) {
                    errorMessage = QString("第 %1 行: ELSE 没有匹配的 IF").arg(cmd.lineNumber);
                    return false;
                }
                break;
                
            case CommandType::EndIf:
                if (ifDepth == 0) {
                    errorMessage = QString("第 %1 行: ENDIF 没有匹配的 IF").arg(cmd.lineNumber);
                    return false;
                }
                ifDepth--;
                break;
                
            case CommandType::For:
                forDepth++;
                lastForLine = cmd.lineNumber;
                break;
                
            case CommandType::EndFor:
                if (forDepth == 0) {
                    errorMessage = QString("第 %1 行: ENDFOR 没有匹配的 FOR").arg(cmd.lineNumber);
                    return false;
                }
                forDepth--;
                break;
                
            default:
                break;
        }
    }
    
    if (ifDepth > 0) {
        errorMessage = QString("第 %1 行: IF 没有匹配的 ENDIF").arg(lastIfLine);
        return false;
    }
    
    if (forDepth > 0) {
        errorMessage = QString("第 %1 行: FOR 没有匹配的 ENDFOR").arg(lastForLine);
        return false;
    }
    
    return true;
}

} // namespace DeviceStudio

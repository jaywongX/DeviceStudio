/**
 * @file iplugin.h
 * @brief 插件接口定义
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QByteArray>
#include <memory>

namespace DeviceStudio {

/**
 * @brief 插件类型枚举
 */
enum class PluginType {
    Protocol,       ///< 协议插件
    Communication,  ///< 通信插件
    View,           ///< 视图插件
    Script,         ///< 脚本插件
    Other           ///< 其他插件
};

/**
 * @brief 插件元数据
 */
struct PluginMetaData {
    QString id;             ///< 插件ID
    QString name;           ///< 插件名称
    QString version;        ///< 插件版本
    QString description;    ///< 描述
    QString author;         ///< 作者
    QString license;        ///< 许可证
    QString website;        ///< 网站
    PluginType type;        ///< 插件类型
    QStringList depends;    ///< 依赖的其他插件
    QStringList provides;   ///< 提供的功能
};

/**
 * @brief 插件基础接口
 * 
 * 所有插件都必须实现此接口
 */
class IPlugin : public QObject
{
    Q_OBJECT

public:
    explicit IPlugin(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPlugin() = default;
    
    // ========== 元数据 ==========
    
    /**
     * @brief 获取插件元数据
     */
    virtual PluginMetaData metaData() const = 0;
    
    /**
     * @brief 获取插件ID
     */
    virtual QString id() const { return metaData().id; }
    
    /**
     * @brief 获取插件名称
     */
    virtual QString name() const { return metaData().name; }
    
    /**
     * @brief 获取插件版本
     */
    virtual QString version() const { return metaData().version; }
    
    /**
     * @brief 获取插件类型
     */
    virtual PluginType type() const { return metaData().type; }
    
    // ========== 生命周期 ==========
    
    /**
     * @brief 初始化插件
     * @return 是否成功
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 关闭插件
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 是否已初始化
     */
    virtual bool isInitialized() const = 0;
    
    // ========== 配置 ==========
    
    /**
     * @brief 获取配置
     */
    virtual QVariantMap configuration() const { return m_configuration; }
    
    /**
     * @brief 设置配置
     */
    virtual void setConfiguration(const QVariantMap& config) { m_configuration = config; }
    
    /**
     * @brief 获取默认配置
     */
    virtual QVariantMap defaultConfiguration() const { return QVariantMap(); }

signals:
    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& error);

protected:
    QVariantMap m_configuration;
};

/**
 * @brief 协议插件接口
 * 
 * 用于扩展协议解析功能
 */
class IProtocolPlugin : public IPlugin
{
    Q_OBJECT

public:
    explicit IProtocolPlugin(QObject* parent = nullptr) : IPlugin(parent) {}
    ~IProtocolPlugin() override = default;
    
    // ========== 协议信息 ==========
    
    /**
     * @brief 获取支持的协议名称列表
     */
    virtual QStringList supportedProtocols() const = 0;
    
    /**
     * @brief 检查是否支持指定协议
     */
    virtual bool supportsProtocol(const QString& protocolName) const = 0;
    
    // ========== 协议解析 ==========
    
    /**
     * @brief 解析数据
     * @param protocolName 协议名称
     * @param rawData 原始数据
     * @param result 解析结果
     * @return 是否成功
     */
    virtual bool parse(const QString& protocolName, const QByteArray& rawData, 
                      QVariantMap& result) = 0;
    
    /**
     * @brief 打包数据
     * @param protocolName 协议名称
     * @param data 数据
     * @param rawData 打包后的原始数据
     * @return 是否成功
     */
    virtual bool pack(const QString& protocolName, const QVariantMap& data, 
                     QByteArray& rawData) = 0;
    
    /**
     * @brief 验证数据
     * @param protocolName 协议名称
     * @param rawData 原始数据
     * @return 是否有效
     */
    virtual bool validate(const QString& protocolName, const QByteArray& rawData) = 0;
    
    /**
     * @brief 获取协议描述
     */
    virtual QString protocolDescription(const QString& protocolName) const = 0;
};

/**
 * @brief 通信插件接口
 * 
 * 用于扩展通信方式
 */
class ICommunicationPlugin : public IPlugin
{
    Q_OBJECT

public:
    explicit ICommunicationPlugin(QObject* parent = nullptr) : IPlugin(parent) {}
    ~ICommunicationPlugin() override = default;
    
    // ========== 通信信息 ==========
    
    /**
     * @brief 获取通信类型标识
     */
    virtual QString communicationType() const = 0;
    
    /**
     * @brief 获取通信类型显示名称
     */
    virtual QString communicationDisplayName() const = 0;
    
    // ========== 连接管理 ==========
    
    /**
     * @brief 连接设备
     * @param config 连接配置
     * @return 是否成功
     */
    virtual bool connect(const QVariantMap& config) = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief 是否已连接
     */
    virtual bool isConnected() const = 0;
    
    // ========== 数据收发 ==========
    
    /**
     * @brief 发送数据
     * @param data 数据
     * @return 发送的字节数,失败返回-1
     */
    virtual qint64 send(const QByteArray& data) = 0;
    
    /**
     * @brief 接收数据
     * @return 接收到的数据
     */
    virtual QByteArray receive() = 0;
    
    /**
     * @brief 获取可读数据大小
     */
    virtual qint64 bytesAvailable() const = 0;
    
    // ========== 配置 ==========
    
    /**
     * @brief 获取配置参数描述
     * @return 参数描述列表
     */
    virtual QVariantMap configParameters() const = 0;
    
    /**
     * @brief 获取当前连接配置
     */
    virtual QVariantMap currentConfig() const = 0;

signals:
    /**
     * @brief 数据接收信号
     */
    void dataReceived(const QByteArray& data);
    
    /**
     * @brief 数据发送信号
     */
    void dataSent(const QByteArray& data);
    
    /**
     * @brief 连接状态改变信号
     */
    void connectionStateChanged(bool connected);
};

/**
 * @brief 视图插件接口
 * 
 * 用于扩展可视化视图
 */
class IViewPlugin : public IPlugin
{
    Q_OBJECT

public:
    explicit IViewPlugin(QObject* parent = nullptr) : IPlugin(parent) {}
    ~IViewPlugin() override = default;
    
    /**
     * @brief 获取视图类型标识
     */
    virtual QString viewType() const = 0;
    
    /**
     * @brief 获取视图显示名称
     */
    virtual QString viewDisplayName() const = 0;
    
    /**
     * @brief 创建视图控件
     * @return 控件指针(调用者负责释放)
     */
    virtual QWidget* createView(QWidget* parent = nullptr) = 0;
    
    /**
     * @brief 更新视图数据
     */
    virtual void updateView(QWidget* view, const QVariantMap& data) = 0;
    
    /**
     * @brief 获取视图配置控件
     * @return 配置控件指针(可选,nullptr表示无配置)
     */
    virtual QWidget* createConfigWidget(QWidget* parent = nullptr) { return nullptr; }
};

// 智能指针类型定义
using IPluginPtr = std::shared_ptr<IPlugin>;
using IProtocolPluginPtr = std::shared_ptr<IProtocolPlugin>;
using ICommunicationPluginPtr = std::shared_ptr<ICommunicationPlugin>;
using IViewPluginPtr = std::shared_ptr<IViewPlugin>;

} // namespace DeviceStudio

// 元类型注册
Q_DECLARE_METATYPE(DeviceStudio::PluginType)
Q_DECLARE_METATYPE(DeviceStudio::PluginMetaData)

# DeviceStudio

一款面向嵌入式开发、物联网开发、硬件工程师的通用设备调试与数据分析平台。

## 功能特性

### 核心功能
- 🔌 **多协议支持**：串口、TCP/UDP、Modbus、CAN总线
- 📊 **实时可视化**：波形图表、仪表盘、数据监控
- 📝 **数据管理**：历史记录、数据导出、回放分析
- 🔧 **脚本引擎**：Lua/JavaScript自动化脚本
- 🧩 **插件系统**：可扩展的插件架构

### 通信协议
- 串口通信 (UART/USART)
- TCP/UDP 网络通信
- Modbus RTU/TCP
- CAN 总线通信

## 技术栈

| 组件 | 技术选型 |
|------|----------|
| 编程语言 | C++17 |
| UI框架 | Qt 6.x / Qt 5.15+ |
| 构建系统 | CMake 3.16+ |
| 编译器 | MSVC 2022 / GCC 11+ |
| 图表库 | QCustomPlot |
| 日志库 | spdlog |
| JSON解析 | nlohmann/json |
| 单元测试 | Google Test |

## 项目结构

```
DeviceStudio/
├── src/
│   ├── main/           # 程序入口
│   ├── core/           # 核心模块（设备管理、数据中心）
│   ├── communication/  # 通信模块（串口、TCP、Modbus、CAN）
│   ├── visualization/  # 可视化模块（图表、仪表盘）
│   ├── ui/             # UI组件（主窗口、面板）
│   ├── plugin/         # 插件系统
│   └── utils/          # 工具模块（日志、配置）
├── tests/              # 单元测试
├── config/             # 配置文件
├── docs/               # 文档
├── cmake/              # CMake配置
└── .github/            # GitHub Actions CI/CD
```

## 快速开始

### 环境要求

#### Windows
- Visual Studio 2022 或更高版本
- CMake 3.16+
- Qt 6.x 或 Qt 5.15+

#### Linux
- GCC 11+ 或 Clang 14+
- CMake 3.16+
- Qt 6.x 或 Qt 5.15+

### 构建步骤

#### Windows
```batch
# 使用提供的构建脚本
build.bat

# 或手动构建
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Linux
```bash
# 使用提供的构建脚本
chmod +x build.sh
./build.sh

# 或手动构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 运行测试

```bash
cd build
ctest --output-on-failure
```

## 开发指南

详细的开发文档请参阅 [docs](docs/) 目录：
- [产品设计文档](docs/产品设计文档.md)
- [接口设计文档](docs/接口设计文档.md)
- [存储设计文档](docs/存储设计文档.md)

## 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE) 文件。

# DeviceStudio 图标资源清单

本文档列出了项目所需的所有图标资源。请下载对应图标并放入 `resources/icons/` 目录。

## 推荐图标库

以下图标库提供高质量的 PNG 图标：

1. **Feather Icons** - https://feathericons.com/ (推荐，简洁现代，可导出PNG)
2. **Material Design Icons** - https://pictogrammers.com/library/mdi/
3. **Fluent UI System Icons** - https://github.com/microsoft/fluentui-system-icons
4. **Iconify** - https://icon-sets.iconify.design/ (可搜索多种图标库并导出PNG)

---

## 必需图标列表

### 应用程序图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `app_icon.png` | 应用程序图标 | cpu, device, settings |

### 工具栏图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `new.png` | 新建项目 | file-plus, document-add |
| `open.png` | 打开项目 | folder-open |
| `save.png` | 保存项目 | save, download |
| `run.png` | 运行脚本 | play, caret-right |
| `stop.png` | 停止脚本 | square, stop |
| `clear.png` | 清空内容 | trash, x, delete |
| `export.png` | 导出数据 | upload, export |

### 面板/标签页图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `terminal.png` | 数据终端 | terminal, console |
| `chart.png` | 曲线图表 | chart-line, activity |
| `gauge.png` | 仪表盘 | gauge, meter, dashboard |
| `script.png` | 脚本编辑 | code, file-code |
| `monitor.png` | 数据监控 | monitor, table |

### 设备相关图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `add_device.png` | 添加设备 | plus-circle, device-add |
| `remove_device.png` | 移除设备 | minus-circle, device-remove |
| `device.png` | 设备 | cpu, hard-drive |
| `serial.png` | 串口设备 | usb, cable |
| `tcp.png` | TCP设备 | wifi, network |
| `udp.png` | UDP设备 | network, broadcast |
| `can.png` | CAN总线 | bus, network |
| `modbus.png` | Modbus | protocol, network |

### 状态图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `connected.png` | 已连接状态 | check-circle, link |
| `disconnected.png` | 已断开状态 | x-circle, unlink |
| `error.png` | 错误状态 | alert-circle, error |
| `warning.png` | 警告状态 | alert-triangle |
| `info.png` | 信息状态 | info |

### 其他图标
| 文件名 | 用途 | 推荐关键词 |
|--------|------|------------|
| `settings.png` | 设置 | settings, cog |
| `refresh.png` | 刷新 | refresh-cw |
| `search.png` | 搜索 | search |
| `folder.png` | 文件夹 | folder |
| `file.png` | 文件 | file |
| `log.png` | 日志 | list, file-text |

---

## 图标格式要求

- **格式**: PNG（代码中使用 `.png` 格式）
- **尺寸**: 建议 24x24 或 32x32 像素
- **背景**: 必须透明背景
- **颜色**: 
  - 推荐 single-color 设计，便于主题适配
  - 使用 #333333 (亮色主题) 或 #e0e0e0 (暗色主题) 作为默认颜色

---

## Feather Icons 对应表

如果使用 Feather Icons，以下是推荐的图标名称：

```
new.png         → file-plus
open.png        → folder-open
save.png        → save
run.png         → play
stop.png        → square
clear.png       → trash-2
export.png      → upload
terminal.png    → terminal
chart.png       → activity
gauge.png       → gauge (需自定义或使用 circle)
script.png      → code
monitor.png     → monitor
add_device.png  → plus-circle
remove_device.png → minus-circle
device.png      → cpu
serial.png      → usb
tcp.png         → wifi
udp.png         → radio
can.png         → git-branch (或自定义)
modbus.png      → database (或自定义)
connected.png   → check-circle
disconnected.png → x-circle
error.png       → alert-circle
warning.png     → alert-triangle
info.png        → info
settings.png    → settings
refresh.png     → refresh-cw
search.png      → search
folder.png      → folder
file.png        → file
log.png         → list
```

---

## 图标下载方法

### 方法1：从 Iconify 下载 PNG

1. 访问 https://icon-sets.iconify.design/feather/
2. 搜索需要的图标
3. 点击图标，选择 "Download PNG"
4. 设置尺寸为 24 或 32，颜色为 #333333
5. 下载后重命名为对应文件名

### 方法2：从 Feather Icons 官网导出

1. 访问 https://feathericons.com/
2. 点击需要的图标
3. 点击 "Download SVG"
4. 使用在线工具转换为 PNG（如 cloudconvert.com）

---

## 验证

图标文件放置完成后，确保以下目录结构：

```
resources/
├── resources.qrc
├── icons/
│   ├── app_icon.png
│   ├── new.png
│   ├── open.png
│   ├── save.png
│   ├── run.png
│   ├── stop.png
│   ├── clear.png
│   ├── export.png
│   ├── terminal.png
│   ├── chart.png
│   ├── gauge.png
│   ├── script.png
│   ├── monitor.png
│   ├── add_device.png
│   ├── remove_device.png
│   ├── device.png
│   ├── serial.png
│   ├── tcp.png
│   ├── udp.png
│   ├── can.png
│   ├── modbus.png
│   ├── connected.png
│   ├── disconnected.png
│   ├── error.png
│   ├── warning.png
│   ├── info.png
│   ├── settings.png
│   ├── refresh.png
│   ├── search.png
│   ├── folder.png
│   ├── file.png
│   └── log.png
├── themes/
│   ├── light.qss
│   └── dark.qss
└── protocols/
    ├── temp_sensor.json
    └── modbus_rtu_template.json
```

重新编译项目后，图标将自动加载。

# T10读卡器配置工具

[![Version](https://img.shields.io/badge/version-1.0.0.1-blue.svg)](https://github.com/jlmay/T10RfSetTool/releases)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://github.com/jlmay/T10RfSetTool)

T10 RF Configuration Tool - 用于配置 T10 系列读卡器的射频参数

## 功能特点

- **射频通信速率设置** - 支持 106K/212K/424K/848K 四种速率
- **WTX 次数设置** - 配置等待时间扩展次数
- **恢复默认参数** - 一键恢复设备出厂设置
- **设备探测** - 自动检测固件支持的功能
- **多种连接方式** - 支持 USB 和串口连接

## 系统要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows 7/8/10/11 (32位或64位) |
| 运行库 | 无需额外安装运行库 |
| 硬件 | T10 系列读卡器 (USB 或串口版本) |

## 快速开始

### 1. 下载程序

从 [Releases](../../releases) 页面下载最新版本的 `T10RfSetTool.zip`，解压到任意目录。

### 2. 连接设备

- 将 T10 读卡器通过 USB 线或串口线连接到电脑
- 运行 `T10SetTool.exe`

### 3. 打开设备

在 **Connection Settings** 区域选择连接类型：

- **USB**: 选择 `USB (port=100)`
- **串口**: 选择 `Serial (COM)`，然后选择对应的 COM 口号和波特率

点击 **Connect** 按钮连接设备。连接成功后，**FW Ver** 框会显示固件版本号。

### 4. 配置参数

| 操作 | 步骤 |
|------|------|
| 设置射频速率 | Operation 选择 `type=0x01 Set RF Rate` → 选择速率 → 点击 **Set Param** |
| 设置 WTX 次数 | Operation 选择 `type=0x03 Set WTX Count` → 输入数值 → 点击 **Set Param** |
| 恢复默认参数 | 点击 **Restore Default** 按钮 |
| 探测支持功能 | 点击 **Probe All** 按钮 |

## 文件说明

```
T10RfSetTool/
├── T10SetTool.exe      # 主程序
├── dcrf32.dll          # 读卡器驱动库
└── 使用说明.md         # 详细使用说明
```

## 射频速率说明

| 速率 | 值 | 说明 |
|------|-----|------|
| 106K | 0x00 | 标准速率，兼容性最好 |
| 212K | 0x11 | 较高速率 |
| 424K | 0x33 | 高速率 |
| 848K | 0x77 | 最高速率，部分卡片可能不支持 |

## 注意事项

> ⚠️ **重要**: 通过本工具设置的参数在设备断电后会丢失！每次上电后需要重新配置，或在应用程序中设置。

- 部分 T10 固件版本可能不支持 Get 操作，如遇 `ret=-2` 错误，说明该功能不被支持
- WTX 次数一般保持默认值即可，仅在特殊卡片需要时调整

## 编译说明

本项目使用 Visual Studio 2022 开发，纯 Win32 API 实现，无需 MFC。

```bash
# 克隆仓库
git clone git@github.com:jlmay/T10RfSetTool.git

# 使用 VS2022 打开 T10SetTool.sln 编译
# 或使用命令行编译:
msbuild T10SetTool.sln /p:Configuration=Release
```

## 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0.0.1 | 2026-03-20 | 初始版本 |

## 许可证

版权所有 (C) 2026

---

如有问题，请提交 [Issue](../../issues) 或联系技术支持。

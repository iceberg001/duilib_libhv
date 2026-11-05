# net_api_test

Postman 风格的轻量级网络接口调试工具，基于 duilib + libhv 开发，支持 HTTP / TCP / UDP 协议调试，并集成 WUP/Jce 协议的升级检测功能。

## 功能概览

| 功能 | 说明 |
|:---|:---|
| HTTP 调试 | GET / POST / PUT / DELETE / PATCH，自定义 Headers 和 Body，10s 超时 |
| TCP 调试 | 短连接同步、短连接异步、长连接（自动重连） |
| UDP 调试 | 发送数据并接收响应 |
| 升级检测 | WUP/Jce 协议，POST 到升级服务器，解析返回版本信息 |

## 技术栈

- **UI 框架**：[duilib](3rdparty/duilib)（Windows RichEdit 2.0 封装，静态库 `duilib_static`）
- **网络库**：[libhv](3rdparty/libhv)（静态库 `libhv`）
- **协议库**：[wup](3rdparty/wup)（header-only WUP/Jce 模板库）
- **构建系统**：CMake + Visual Studio 2022 + x64 + /MT 静态运行时

## 项目结构

```
net_api_test/
├── CMakeLists.txt              # 主构建文件
├── config.ini                  # 升级检测配置（guid / uid）
├── pack_resources.py           # 资源打包脚本（res/ → resources.zip）
├── 3rdparty/
│   ├── duilib/                 # UI 库源码
│   ├── libhv/                  # 网络库源码（含 include/servercommon 目录联接）
│   └── wup/                    # WUP/Jce 协议头文件
│       └── jce/
│           ├── plugin/taf.h    # WUP 入口头
│           └── Upgrade.h       # JCE 生成的升级接口定义
├── src/
│   ├── main.cpp                # WinMain 入口
│   ├── MainFrame.h/cpp         # 主窗口（HTTP/TCP/UDP/升级检测 UI 逻辑）
│   ├── UpgradeClient.h/cpp     # 升级检测封装（异步调用 + 主线程回调）
│   ├── resource.rc             # Windows 资源（图标 + 嵌入 zip）
│   └── resources.zip           # 打包后的 UI 资源（自动生成）
├── res/
│   ├── skin.xml                # UI 布局（深色主题）
│   ├── app.ico                 # 应用图标
│   └── image/                  # UI 图片资源
└── build/                      # 构建输出
```

## 构建方法

### 前置条件

- Visual Studio 2022（含 C++ 桌面开发工作负载）
- CMake 3.15+
- Python 3（用于资源打包脚本）

### 编译步骤

```powershell
# 配置（首次）
cmake -S . -B build -A x64

# 编译 Release
cmake --build build --config Release --target net_api_test

# 可执行文件
# build\Release\net_api_test.exe
# build\Release\config.ini          # 升级检测配置
```

## 使用说明

### HTTP 调试

1. 选择 HTTP 标签页
2. 选择请求方法（GET / POST / PUT / DELETE / PATCH）
3. 输入 URL
4. 在 Headers 区域输入请求头（每行 `key: value` 格式）
5. 在 Body 区域输入请求体
6. 点击「发送」按钮

### TCP 调试

1. 选择 TCP 标签页
2. 输入 Host 和 Port
3. 选择连接模式：
   - **短连接同步**：阻塞发送并等待响应（5s 超时）
   - **短连接异步**：异步发送，回调返回响应
   - **长连接**：建立持久连接，自动重连（1s 间隔，最多 3 次）
4. 输入发送数据
5. 点击「发送」按钮

### UDP 调试

1. 选择 UDP 标签页
2. 输入 Host、Port 和发送数据
3. 点击「发送」按钮
4. 工具自动等待 2s 接收响应后关闭

### 升级检测

1. 在响应区域上方的输入框中确认升级服务器 URL（默认 `https://prx.upchina.com/wup/`）
2. 点击「检测升级」按钮
3. 工具自动完成：
   - 获取本机 MAC 地址（作为 `deviceID` 和 `mac`）
   - 动态构建 `xua`（屏幕分辨率 + OS 版本）
   - 从 `config.ini` 读取 `guid` 和 `uid`
   - WUP 编码 `CheckUpgradeReq` → HTTP POST → WUP 解码 `CheckUpgradeRsp`
4. 响应结果：
   - `rsp.ret = 0`：有新版本，显示下载地址、版本号、MD5 等
   - `rsp.ret = 1`：无新版本（`EUEC_NO_NEW_VER`）

#### config.ini 配置

```ini
[Upgrade]
; guid - 用户唯一标识（从注册表获取，首次留空）
guid=
; uid - 业务账号 ID（留空表示无）
uid=
```

`config.ini` 位于可执行文件同目录，CMake 构建后自动复制到输出目录。

## 升级检测接口封装

升级检测已封装为 `UpgradeClient` 类，支持异步调用 + 主线程回调：

```cpp
#include "UpgradeClient.h"

// 异步调用，回调在主线程执行
UpgradeClient::CheckUpgrade(hwnd, params, [](const UpgradeResult& r) {
    if (r.hasNewVersion) {
        // r.downURL, r.verName, r.packageMd5 ...
    }
});
```

### 接口说明

| 组件 | 说明 |
|:---|:---|
| `UpgradeParams` | 输入参数：`url`、`guid`、`uid` |
| `UpgradeResult` | 输出结果：`success`、`hasNewVersion`、`downURL`、`verName` 等 |
| `UpgradeClient::CheckUpgrade` | 异步发起检测，子线程执行 HTTP 请求，完成后 `PostMessage` 回主线程 |
| `UpgradeClient::HandleResult` | 主线程消息处理，调用回调函数 |

### 内部流程

```
CheckUpgrade(hwnd, params, callback)
  │
  ├─ 子线程：GetLocalMac() → BuildXua() → WUP encode → HTTP POST → WUP decode
  │
  └─ PostMessage(WM_UPGRADE_RESULT)
        │
        └─ 主线程：HandleResult() → callback(result)
```

## 编码注意事项

- duilib 头文件为 **GBK 编码**，项目不使用 `/utf-8` 编译选项
- `MainFrame.cpp` 为 **UTF-8 with BOM**，中文字符串字面量被 MSVC 编为 GBK 执行字符集
- `GetCtrlText()` 返回 **UTF-8** 编码的 `std::string`
- **中文字符串比较**必须使用宽字符（`CDuiString` + `_T()`），不能用 `std::string == "中文"`
- **设置中文文本**必须用 `ctrl->SetText(_T("中文"))`，不能用 `SetCtrlText("name", "中文")`

## 参考来源

升级检测逻辑参考 `F:\workspace\AroundSoft\misc\Update\WupCheck.cpp`：
- `deviceID` 和 `mac` 使用本机 MAC 地址（`GetLocalMac()`）
- `xua` 动态构建（屏幕分辨率、OS 版本、渠道号等）
- 响应检查 `rsp.ret`：`0` = 有新版本，`1` = 无新版本
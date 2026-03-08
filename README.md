# GNSS-SPP-SPV-OEM7
基于 C++ 的 GNSS 单点定位与测速（SPP/SPV）课程实践项目，针对 GPS+BDS，支持解析 NovAtel OEM7 二进制数据（RANGEB/EPHEM/PSRPOS 等），实现观测预处理与 GF/MW 异常检测、广播星历轨道与钟差传播、对流层模型以及基于 Eigen 的最小二乘位置/速度解算，提供离线文件和实时 TCP 流两种输入模式。

项目完整实验设计与结果可参考：[项目实验报告（PDF）](项目实验报告.pdf)

## 项目简介

`SPP_SPV` 是一个基于 C++ 的 GNSS 单点定位 / 测速（Single Point Positioning / Single Point Velocity, SPP/SPV）示例程序。  
项目支持 **GPS + BDS** 广播星历，输入为 **NovAtel OEM7 二进制数据流**（文件或 TCP 网络流），完成从原始观测数据到三维位置和速度的整条处理链。

- **输入**：NovAtel OEM7 原始二进制流（包含 RANGEB、GPSEPHEM、BDSEPHEM、PSRPOS 等消息）。  
- **输出**：接收机在 ECEF/BLH 坐标系下的位置、速度，以及与 OEM 自身解算结果的 ENU 误差等文本结果。

## 功能特性

- **单点定位 / 测速 (SPP/SPV)**  
  - 基于广播星历的单点定位（SPP），支持 GPS + BDS 双系统联合解算。  
  - 基于多普勒观测的单点测速（SPV），解算三维速度与钟漂移。

- **文件 / 实时双模式输入**  
  - **文件模式**：从本地二进制数据文件（如 `oem719-202504011900-1.bin`）读取 OEM7 数据。  
  - **实时模式**：通过 TCP Socket 从远端 OEM7 接收机获取实时数据流。

- **观测预处理与异常检测**  
  - 计算 **GF（Geometry-Free）/ MW（Melbourne–Wübbena）组合**。  
  - 基于历元间 GF/MW 差分进行周跳 / 异常探测与平滑。  
  - 支持电离层消除伪距 (PIF) 组合。

- **广播星历轨道与钟差传播**  
  - 实现 GPS / BDS 广播星历解析与轨道传播。  
  - 包括 GEO/MEO/IGSO 不同轨道类型处理、相对论效应改正、地球自转改正等。

- **对流层与坐标转换**  
  - Hopfield 对流层模型。  
  - ECEF ↔ BLH、ECEF ↔ ENU/NEU 坐标转换。  
  - 计算与 OEM 自身位置解之间的 ENU 误差。

## 目录结构（简要）

```text
SPP_SPV/
  ├─ src/
  │   ├─ SPP.h                  # 核心常量、数据结构与函数声明
  │   ├─ main.cpp               # 程序入口，文件/实时模式选择与主流程
  │   ├─ Decode.cpp             # NovAtel OEM7 二进制解析 (RANGEB/EPHEM/PSRPOS)
  │   ├─ ErrorCorrection.cpp    # GF/MW 组合与异常检测、对流层模型
  │   ├─ SatellitePosition.cpp  # 广播星历轨道/钟差传播
  │   ├─ CoordinateTransformation.cpp  # 坐标转换 (ECEF/BLH/ENU)
  │   ├─ PVT.cpp                # SPP/SPV 解算 (基于 Eigen)
  │   ├─ Socket.h               # TCP Socket 简单封装
  │   ├─ Matrix.cpp             # 通用矩阵运算（当前主流程未使用）
  │   └─ TimeConverter.cpp      # 时间表示转换（当前主流程未使用）
  ├─ README.md
  ├─ 项目实验报告.pdf          # 项目实验设计、数据与结果说明
  ├─ LICENSE
  └─ .gitignore
```

## 环境与依赖

- **操作系统**
  - Windows 10 及以上（原始代码以 Windows + Winsock 为主测试环境）。  
  - 理论上可移植到 Linux / macOS，但需要适配 Socket 与 Sleep 等平台相关接口。

- **编译器 / 工具链**
  - 推荐：Visual Studio 2019/2022（MSVC, C++11 或以上）。  
  - 或：CMake + GCC / Clang（需自行配置 Socket 与链接选项）。

- **第三方库**
  - [Eigen](https://eigen.tuxfamily.org/)  
    - 用于最小二乘解算和向量 / 矩阵运算。  
    - 建议使用 3.3+ 版本。

- **系统库**
  - Windows：`ws2_32.lib`（Winsock2，用于 TCP Socket）。  
  - POSIX：`libsocket` / `libpthread` 等（若进行移植时使用）。

## 编译方法示例

> 以下以 **Windows + Visual Studio** 为例，其他平台可使用 CMake 自行配置。

1. 安装 Eigen（如解压至 `C:\eigen-3.4.0`）。  
2. 在 VS 中创建 Win32 控制台项目，将 `src/` 目录下所有 `.cpp` 文件加入工程。  
3. 在项目属性中：
   - C/C++ → 常规 → **附加包含目录**：加入 Eigen 头文件路径，例如：`C:\eigen-3.4.0`。  
   - 链接器 → 输入 → **附加依赖项**：加入 `ws2_32.lib`（Windows）。  
4. 确保项目使用 **多字节或 Unicode 字符集** 时，对源码中的中文注释和路径进行相应配置。

如果使用 **CMake**，可以参考类似结构：

```cmake
cmake_minimum_required(VERSION 3.10)
project(SPP_SPV CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(Eigen3 REQUIRED)

add_executable(SPP_SPV
    src/main.cpp
    src/Decode.cpp
    src/ErrorCorrection.cpp
    src/SatellitePosition.cpp
    src/CoordinateTransformation.cpp
    src/PVT.cpp
    src/Matrix.cpp
    src/TimeConverter.cpp
)

target_include_directories(SPP_SPV PRIVATE ${EIGEN3_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(SPP_SPV PRIVATE ws2_32)
endif()
```

> 注：上述 CMake 示例仅供参考，具体源文件列表和平台宏可能需要根据实际工程调整。

## 运行方式

编译得到可执行程序（例如 `SPP_SPV.exe`）后，可以通过交互或命令行参数选择运行模式。

### 1. 输入数据格式（OEM7 二进制）

程序假定输入为 **NovAtel OEM7 原始二进制流**，其中包含至少以下消息类型：

- `RANGEB`（消息 ID 43）：多频码相观测值；  
- `GPSEPHEM`（消息 ID 7）：GPS 广播星历；  
- `BDSEPHEM`（消息 ID 1696）：BDS 广播星历；  
- `PSRPOS`（消息 ID 42）：OEM 自身单点位置解（用于对比 ENU 误差）。

原始流可以来自：

- 本地文件，例如：`oem719-202504011900-1.bin`；  
- 实时网络流（TCP Socket）输出。

### 2. 文件模式

在默认实现中，程序会：

1. 询问运行模式（0：文件模式，1：实时模式）。  
2. 当选择 **0** 时，从当前目录下的 `oem719-202504011900-1.bin` 读取数据。  
3. 逐帧解析 OEM7 数据并运行整条 SPP/SPV 管线。  
4. 结果输出到 `output_file.txt`。

> 建议在开源版本中，将输入文件名、输出路径改为命令行参数或配置文件，而不是硬编码在源码中。

### 3. 实时模式

当选择 **1** 时，程序会：

1. 使用 `Socket.h` 中的封装与预设 IP/端口（如 `47.114.134.129:7190`）建立 TCP 连接；  
2. 周期性接收 OEM7 二进制数据，并对每个历元进行 SPP/SPV 解算；  
3. 将主要结果打印到控制台，并记录到 `output_net.txt`。

在开源部署时，建议将 IP/端口改为可配置项或示例值，例如通过命令行或配置文件传入。

## 输出结果说明

- `output_file.txt` / `output_net.txt` 中通常包含：
  - 每颗卫星的 ECEF 位置 / 速度、钟差 / 钟漂移、仰角 / 方位角等中间结果；  
  - 每历元的接收机 ECEF/BLH 位置与速度；  
  - GPS / BDS 卫星数量、PDOP、位置/速度标准差估计；  
  - 与 OEM 自身位置解之间的 ENU 误差。

文件格式为 **人类可读文本**，可使用脚本（Python/Matlab/Octave 等）进行进一步绘图或分析。

## 许可证

本项目采用 **MIT License** 进行开源，详见根目录下的 `LICENSE` 文件。


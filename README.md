# GNSS-SPP-SPV-OEM7
基于 C++ 的 GNSS 单点定位与测速（SPP/SPV）课程实践项目，针对 GPS+BDS，支持解析 NovAtel OEM7 二进制数据（RANGEB/EPHEM/PSRPOS 等），实现观测预处理与 GF/MW 异常检测、广播星历轨道与钟差传播、对流层模型以及基于 Eigen 的最小二乘位置/速度解算，提供离线文件和实时 TCP 流两种输入模式。

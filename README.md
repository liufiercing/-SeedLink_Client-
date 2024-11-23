
# SeedLink Client with TCP Server

一个基于C语言的 SeedLink 客户端程序，可以接收地震数据并通过 TCP 服务器转发给其他客户端。

## 功能特点

- 支持连接 SeedLink 服务器接收实时地震数据
- 支持请求多通道数据（如 BHZ、BHN、BHE 三分量）
- 内置 TCP 服务器，支持数据转发
- 自动保存 miniSEED 格式数据
- 支持多客户端同时连接
- 详细的日志输出

## 编译

需要的环境：
- GCC
- pthread 库
- Linux/Unix 系统

编译命令：
```
gcc .c -o seedlink_client -pthread
```

## 配置说明

主要配置参数在头文件中定义：
- SEEDLINK_SERVER: SeedLink 服务器地址（默认：rtserve.iris.washington.edu）
- SEEDLINK_PORT: SeedLink 服务器端口（默认：18000）
- SERVER_PORT: TCP 服务器监听端口（默认：8000）
- MAX_CLIENTS: 最大客户端连接数（默认：10）

## 数据格式

### SeedLink 数据包格式
- 8字节 SeedLink 头
  - 2字节：标识符 "SL"
  - 6字节：十六进制序列号
- 512字节 miniSEED 数据

### miniSEED 头格式（48字节）
- 6字节：序列号
- 1字节：数据质量标识
- 1字节：保留字节
- 5字节：台站代码
- 2字节：位置标识
- 3字节：通道代码
- 2字节：网络代码
- 其他时间和控制信息

## 文件说明

- main.c: 主程序入口，处理命令行参数和主循环
- seedlink.h/c: SeedLink 协议实现，包括连接和数据包处理
- miniseed.h/c: miniSEED 格式处理，包括头部解析和数据保存
- server.h/c: TCP 服务器实现，支持多客户端连接和数据转发

## 注意事项

1. 确保有足够的磁盘空间存储数据
2. 检查防火墙设置，确保端口可访问
3. 建议使用稳定的网络连接
4. 数据文件按通道分别保存，格式为：network_station_location_channel.mseed

## 开发计划

- [ ] 添加配置文件支持
- [ ] 改进错误处理机制
- [ ] 添加数据压缩选项
- [ ] 添加 Web 界面
- [ ] 支持更多数据格式

## 参考资料

- [SeedLink 协议文档](https://ds.iris.edu/ds/nodes/dmc/services/seedlink/)
- [miniSEED 格式规范](https://ds.iris.edu/ds/nodes/dmc/data/formats/miniseed/)
- [IRIS 数据中心](https://ds.iris.edu/)
EOF
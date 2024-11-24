# MiniSEED Reader

一个用于读取和解析 MiniSEED 格式地震数据的 C 语言程序。

## 功能特性

- 支持 MiniSEED 格式数据的读取和解析
- 支持 Steim2 压缩格式的解压缩
- 支持多个512字节记录的处理
- 输出解压后的波形数据到 MAT 文件
- 详细的日志输出和错误处理

## 编译方法 

如果不提供文件路径参数，程序将默认读取当前目录下的 `II_BFO_00_BHE1.mseed` 文件。

## 输出格式

程序会输出以下信息：
1. MSEED 头部信息，包括：
   - 台站信息 (NET.STA.LOC.CHN)
   - 时间戳
   - 采样率
   - 数据质量标志
   - 其他控制信息
2. 解压缩进度
3. 最终解压的样本数量

解压后的数据将保存在 `out.mat` 文件中。

## 文件结构

- `read_mseed.c/h`: 主程序和文件读取功能
- `mseed_header.c/h`: MSEED 头部解析功能
- `steim2.c/h`: Steim2 压缩格式解压缩功能
- `analyze_mseed.py`: 用于验证结果的 Python 脚本

## 依赖项

- C 标准库
- Python (用于验证脚本，需要 ObsPy 库)

## 注意事项

1. 程序假设输入的 MiniSEED 文件使用 Steim2 压缩格式
2. 默认处理大端序数据
3. 每个记录大小固定为 512 字节

## 作者

[Your Name]

## 许可证

[License Information]

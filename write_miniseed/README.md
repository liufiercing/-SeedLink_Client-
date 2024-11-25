# MiniSEED Writer

这是一个用C语言实现的MiniSEED格式文件写入工具。支持生成标准的MiniSEED V2.4格式数据文件，包含完整的头部信息、blockettes和Steim2压缩的数据。

## 功能特点

- 支持MiniSEED V2.4格式
- 实现Steim2数据压缩算法
- 支持Blockette 1000和1001
- 自动计算记录长度和帧数
- 支持大端/小端字节序
- 包含数据验证和对比工具

## 文件结构

- `main.c` - 主程序入口，示例代码
- `mseed_header.h/c` - MiniSEED头部结构定义和相关函数
- `blockette.h/c` - Blockette结构定义和处理函数
- `steim2.h/c` - Steim2压缩算法实现
- `utils.h/c` - 工具函数
- `create_mseed.py` - Python对比测试脚本（使用ObsPy）

## 编译方法 
```bash
gcc *.c -Wall -o mseed_writer
```

## 使用方法

1. 编译程序：`gcc *.c -Wall -o mseed_writer`
2. 运行程序：`./mseed_writer`
3. 查看输出：`test.mseed`


3. 程序会生成一个名为 `test.mseed` 的文件，包含示例数据。

## 示例数据说明

默认生成的示例数据包含：
- 台站代码：BJSHS
- 通道代码：BHZ
- 位置代码：00
- 网络代码：BJ
- 采样率：100Hz
- 数据质量：D
- 数据格式：Steim2压缩
- 记录长度：自动计算（2^N字节）
- 时间戳：2024-03-15 14:30:00.0000

## Python对比测试

包含一个使用ObsPy库的Python脚本，用于生成对比用的MiniSEED文件：
```bash
python create_mseed.py
```

## 依赖项

- C标准库
- Python 3.x（可选，用于对比测试）
- ObsPy库（可选，用于对比测试）

## 注意事项

1. 字节序：
   - 头部和blockettes使用大端序写入
   - 数据部分使用小端序写入

2. 记录长度：
   - 自动计算，确保足够容纳所有数据
   - 支持2^8到2^20字节的记录长度

3. 数据压缩：
   - 使用Steim2算法
   - 每帧最多压缩7个样本

## 作者

[liufeng]

## 许可证

[MIT]

## 参考资料

- IRIS SEED Manual V2.4
- Steim2压缩算法文档  
- ObsPy文档
- 开源库：https://github.com/EarthScope/libmseed
感谢开源库的贡献者们！
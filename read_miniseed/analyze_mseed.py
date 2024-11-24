from obspy import read
import numpy as np
import matplotlib.pyplot as plt

def analyze_mseed(filename):
    # 读取mseed文件
    st = read(filename)
    
    # 获取第一个trace
    tr = st[0]
    
    # 打印基本信息
    print("\n=== 文件基本信息 ===")
    print(f"网络台站: {tr.stats.network}.{tr.stats.station}")
    print(f"位置通道: {tr.stats.location}.{tr.stats.channel}")
    print(f"采样率: {tr.stats.sampling_rate} Hz")
    print(f"开始时间: {tr.stats.starttime}")
    print(f"数据点数: {len(tr.data)}")
    
    # 打印前20个数据点
    print("\n=== 前20个数据点 ===")
    for i, value in enumerate(tr.data[:20]):
        print(f"[{i}] {int(value)}")  # 转换为整数以便与C程序对比
    
    # 计算一些基本统计信息
    print("\n=== 数据统计 ===")
    print(f"最小值: {int(np.min(tr.data))}")
    print(f"最大值: {int(np.max(tr.data))}")
    print(f"平均值: {int(np.mean(tr.data))}")
    print(f"标准差: {int(np.std(tr.data))}")

    # 绘制波形图
    plt.figure(figsize=(12, 6))
    plt.plot(tr.data[:100])  # 绘制前100个点
    plt.title(f"{tr.stats.network}.{tr.stats.station}.{tr.stats.location}.{tr.stats.channel}")
    plt.xlabel("Sample Number")
    plt.ylabel("Counts")
    plt.grid(True)
    plt.savefig("waveform.png")
    plt.close()

if __name__ == "__main__":
    analyze_mseed("II_BFO_00_BHE1.mseed") 
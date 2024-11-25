from obspy import UTCDateTime, Stream, Trace, read
import numpy as np

# 创建与C程序相同的数据
sampling_rate = 20  # 采样率20Hz
npts = 200  # 采样点数
t = np.arange(npts) / sampling_rate
data = 20000.0 * np.sin(2.0 * np.pi * t * (sampling_rate/npts))
data = data.astype(np.int32)  # 转换为int32类型

# 创建时间戳 (2024-03-15 14:30:00.0000)
starttime = UTCDateTime(2024, 3, 15, 14, 30, 0) + 0.0000

# 创建Trace对象
stats = {
    'network': 'BJ',
    'station': 'BJSHS',
    'location': '00',
    'channel': 'BHZ',
    'sampling_rate': sampling_rate,
    'starttime': starttime,
    'mseed': {
        'dataquality': 'D',
        'record_length': 512,
        'encoding': 11,
        'byteorder': 1,  # 使用大端序
        # 先定义blkt1000
        'blkt1000': {
            'encoding': 11,
            'byteorder': 1,  # 大端序
            'reclen': 9,
            'next_blockette': 56  # 指向下一个blockette的位置
        },
        # 后定义blkt1001
        'blkt1001': {
            'timing_quality': 100,
            'microsecond': 38,
            'frame_count': 7,
            'next_blockette': 0   # 最后一个blockette
        }
    }
}

tr = Trace(data=data, header=stats)

# 创建Stream对象
st = Stream([tr])

# 写入miniSEED文件
st.write('obspy_test.mseed', format='MSEED', 
         reclen=512,  # 记录长度
         encoding=11,  # Steim2压缩
         byteorder=1,  # 改回1，表示小端序
         blockette_order=['1000', '1001'])

print("已创建obspy_test.mseed文件")

# 读取并打印文件信息进行验证
st_read = read('obspy_test.mseed')
print("\n文件信息:")
print(st_read)
print("\n详细信息:")
print(st_read[0].stats) 
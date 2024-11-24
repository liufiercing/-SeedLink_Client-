% 读取数据
fid = fopen('out.mat', 'rb');
data = fread(fid, inf, 'int32');
fclose(fid);

% 绘图
figure;
plot(data);
title('Decompressed Waveform');
xlabel('Sample Number');
ylabel('Amplitude');
grid on;
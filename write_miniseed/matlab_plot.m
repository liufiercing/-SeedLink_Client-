% 读取原始样本数据
fid = fopen('samples.dat', 'rb');
data = fread(fid, [200, 1], 'int32');
fclose(fid);

% 绘制波形
figure;
plot(data);
grid on;
title('Original Samples');
xlabel('Sample Number');
ylabel('Amplitude');
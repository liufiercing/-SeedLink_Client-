% ��ȡԭʼ��������
fid = fopen('samples.dat', 'rb');
data = fread(fid, [200, 1], 'int32');
fclose(fid);

% ���Ʋ���
figure;
plot(data);
grid on;
title('Original Samples');
xlabel('Sample Number');
ylabel('Amplitude');
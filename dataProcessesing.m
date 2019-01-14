fs = 10000;

deltaT = 1/fs;

fc = 50;

RC = 1/(2*pi*fc);

alpha = deltaT/(RC+deltaT);


for i = 1:500
    x(i) = cos(fc*i)*3.3/2+3.3/2;
end

y = 0;
for i = 2:500
    y(i) = alpha*x(i)+(1-alpha)*y(i-1);
end

figure(1)
hold on
plot(x)
plot(y)
hold off
%%
X = fftshift(fft(x));
Y = fftshift(fft(y));

%%
z = tf('z');

H = alpha/((1-alpha)*z);



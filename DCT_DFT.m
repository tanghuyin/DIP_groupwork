clear;
clc;
I = imread("D:\Twinkle\junior_first_half_year\DIP\DIP_groupwork\pic2.jpg");
% I = imread("pic2.jpg")
I = rgb2gray(I);  

DCT = dct2(I);
DCT = abs(DCT);
DFT = fft2(I);
DFT = abs(DFT)

% imshow(uint8(DCT))
% title("DCT");
imshow(uint16(DFT))
title("DFT");
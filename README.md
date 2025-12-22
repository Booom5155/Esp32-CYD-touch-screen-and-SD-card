# Esp32-CYD-touch-screen-and-SD-card
A simple repo for usage of CYD touchscreen and SD card in the same program, though not simultaneously. This is a very simplistic and non optimal implementation that allows for switching between touch screen and SD, but for those who just want a functioning way to use both this should work for you. The specific place that I got my CYD from is here [esp32 CYD](https://www.amazon.com/dp/B0DDPY97JC/?coliid=ID2RTVFWTVI86&colid=2A45AGQM1QYFO&ref_=list_c_wl_lv_ov_lig_dp_it&th=1) on amazon.

In the pixelNormalize function of the OS class you should add your own screen calibration numbers. I would recommend using [this tutorial](https://randomnerdtutorials.com/esp32-cheap-yellow-display-cyd-resistive-touchscreen-calibration/) to get these values as they change for each board. 

## Notes:

-WARNING: This pixel normalization does not work for other rotations than 1.

-The "Copy to SD" folder is the entire contents that I had on my SD when I ran my tests. 

-When I uploaded this code to the board in the Arduino IDE I used the "ESP32 Dev Module" option.

## Credits: 
### This repo is the combination of many tutorials, here is one of the foremost:

[SD card](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

[TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder/tree/master)


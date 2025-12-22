#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TJpg_Decoder.h>
#include "FS.h"
#include <SD.h>

#include "OS.h"
#include "SDFunctions.h"


#define XPT2046_IRQ 36
#define XPT2046_CS 33

SPIClass mySpiScreen(HSPI);
SPIClass mySpiSD(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

OS device(&tft,&ts,&mySpiScreen,&mySpiSD,&tft_output);
int time_on = 0;
int xMax = 0;
int xMin = 500;
int yMin = 500;
int yMax = 0;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("Begin");

  device.setup();
  

  // listDir(SD, "/", 0);
}

void resetScreen(){
  device.tft->fillScreen(TFT_BLACK);
  // Clear the screen before writing to it
}


void printTouchToDisplay(TS_Point p) {

  // Clear screen first
  resetScreen();
  device.tft->setTextColor(TFT_WHITE, TFT_BLACK);

  int x = 320 / 2; // center of display
  int y = 100;
  int fontSize = 2;

  String temp = "X = " + String(device.pixelNormalize(p).x);
  device.tft->drawCentreString(temp, x, y, fontSize);

  y += 16;
  temp = "Y = " + String(device.pixelNormalize(p).y);
  device.tft->drawCentreString(temp, x, y, fontSize);

}

void printTouchToSerial(TS_Point p) {
  Serial.print("Pressure = ");
  Serial.print(p.z);
  Serial.print(", x = ");
  Serial.print(p.x);
  Serial.print(", y = ");
  Serial.print(p.y);
  Serial.println();
}


void loop(){

  if (device.ts->tirqTouched() && device.ts->touched()) {
    TS_Point p = device.ts->getPoint();

    printTouchToSerial(p);
    printTouchToDisplay(p);
    Location temp = device.pixelNormalize(p);
    device.tft->drawPixel(temp.x, temp.y, TFT_RED);

    device.activateSDcard();
    uint16_t w = 0, h = 0;
    TJpgDec.getFsJpgSize(&w, &h, "/Portal.jpg", SD);
    TJpgDec.setJpgScale(8);
    TJpgDec.drawFsJpg((temp.x)-(w/(2*8)), (temp.y)-(h/(2*8)), "/Portal.jpg", SD);
    device.activateTouchscreen();
    
    delay(100);
  }
}

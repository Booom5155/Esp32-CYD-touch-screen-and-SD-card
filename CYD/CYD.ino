#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TJpg_Decoder.h>
#include "FS.h"
#include <SD.h>
#include "esp32/himem.h"
#include "esp_psram.h"

#include "OS.h"
#include "SDFunctions.h"


#define XPT2046_IRQ 36
#define XPT2046_CS 33

SPIClass mySpiScreen(HSPI);
SPIClass mySpiSD(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

OS device(&tft,&ts,&mySpiScreen,&mySpiSD,&tft_output);

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


// For holding the image on the heap
uint8_t* jpgBuffer = nullptr;
size_t fileSize = 0;

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("Begin");

  device.setup();
  device.setRotation(0);

  fileSize = device.loadFile(&jpgBuffer, "/Portal.jpg", SD);
  // listDir(SD, "/", 0);
}

void resetScreen(){
  device.tft->fillScreen(TFT_BLACK);
  // Clear the screen before writing to it
}


void printTouchToDisplay(TS_Point p) {
  device.tft->setTextColor(TFT_WHITE, TFT_BLACK);

  int x = device.getWidth() / 2; // center of display
  int y = 16;//(device.getHeight() / 2) - 16;
  int fontSize = 2;

  String temp = "X = " + String(device.pixelNormalize(p).x);
  device.tft->drawCentreString(temp, x, y, fontSize);

  temp = "Y = " + String(device.pixelNormalize(p).y);
  y += 16; device.tft->drawCentreString(temp, x, y, fontSize);
  y += 16; device.tft->drawCentreString("Total heap: "+String(ESP.getHeapSize()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("Free heap: "+String(ESP.getFreeHeap()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("Total PSRAM: "+String(ESP.getPsramSize()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("Free PSRAM: "+String(ESP.getFreePsram()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("psram size "+String(esp_psram_get_size()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("himem free "+String(esp_himem_get_free_size()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("himem phys "+String(esp_himem_get_phys_size()), x, y, fontSize);
  y += 16; device.tft->drawCentreString("himem reserved "+String(esp_himem_reserved_area_size()), x, y, fontSize);
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
    TS_Point p = device.getPointTouched();
    Location point = device.pixelNormalize(p);
    device.tft->drawPixel(point.x, point.y, TFT_RED);

    if(point.x< 25 && point.y< 25){
      device.setRotation(device.getRotation()+1);
    }

    resetScreen();
    printTouchToSerial(p);
    printTouchToDisplay(p);
    device.tft->drawRect(0,0,25,25,TFT_RED);

    // Use preloaded jpg
    uint16_t w = 0, h = 0;
    TJpgDec.getJpgSize(&w, &h, jpgBuffer, fileSize);
    TJpgDec.setJpgScale(8);
    TJpgDec.drawJpg((point.x)-(w/(2*8)), (point.y)-(h/(2*8)), jpgBuffer, fileSize);

    // Pull jpg straight from SD (Slower)
    // device.activateSDcard();
    // uint16_t w = 0, h = 0;
    // TJpgDec.getFsJpgSize(&w, &h, "/Portal.jpg", SD);
    // TJpgDec.setJpgScale(8);
    // TJpgDec.drawFsJpg((point.x)-(w/(2*8)), (point.y)-(h/(2*8)), "/Portal.jpg", SD);
    // device.activateTouchscreen();
    
    // delay(100);
  }
}

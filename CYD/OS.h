#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <BasicLinearAlgebra.h>
#include <TJpg_Decoder.h>
#include <SD.h>
#include "FS.h"

#define SD_CS 5
#define SD_SCK 18
#define SD_MOSI 23
#define SD_MISO 19

// #define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

class Location{
    public:
        Location(int x, int y) {
            this->x = x;
            this->y = y;
        }
        int x;
        int y;
};


class OS{
    public:
        OS(TFT_eSPI *tftIn, XPT2046_Touchscreen *tsIn, SPIClass *mySpiScreenIn, SPIClass *mySpiSDIn, bool (*tft_output_in)(int16_t, int16_t, uint16_t, uint16_t, uint16_t*)){
            this->tft = tftIn;
            this->ts = tsIn;
            this->mySpiScreen = mySpiScreenIn;
            this->mySpiSD = mySpiSDIn;
            this->tft_output = tft_output_in;
            this->useTouchscreen = false;
        }


        void setup(){
            
            //Init SD card
            this->mySpiSD->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
            
            if (!SD.begin(SD_CS, *(this->mySpiSD))) {
                Serial.println(F("SD.begin failed!"));
                while (1) delay(0);
            }
            Serial.println("SD card initialized.");

            
            //Init tft
            this->tft->init();            
            this->tft->invertDisplay(1);
            this->tft->setSwapBytes(true);

            this->setRotation(1);
            this->tft->fillScreen(TFT_BLACK);

    
            //Set Logo screen
            TJpgDec.setJpgScale(1);
            TJpgDec.setCallback(tft_output);

            uint16_t w = 0, h = 0;
            TJpgDec.getFsJpgSize(&w, &h, "/Portal.jpg", SD);
            Serial.println(String(w)+":"+String(h));
            TJpgDec.drawFsJpg((320/2)-(w/2), (240/2)-(h/2), "/Portal.jpg", SD);

            
            activateTouchscreen();
        }

        TFT_eSPI *tft;
        XPT2046_Touchscreen *ts;
        SPIClass *mySpiScreen;
        SPIClass *mySpiSD;
        
        bool (*tft_output)(int16_t, int16_t, uint16_t, uint16_t, uint16_t*);


        Location pixelNormalize(TS_Point p){
            int x,y;
            //240 (width) and 320 (height)

            float alpha_x, beta_x, alpha_y, beta_y, delta_x, delta_y;

            // // REPLACE WITH YOUR OWN CALIBRATION VALUES » https://RandomNerdTutorials.com/touchscreen-calibration/
            alpha_x = 0.088;
            beta_x = -0.001;
            delta_x = -15.824;
            alpha_y = 0.001;
            beta_y = -0.072;
            delta_y = 265.126;


            x = alpha_y * p.x + beta_y * p.y + delta_y;
            // clamp x between 0 and SCREEN_WIDTH - 1
            x = max(0, x);
            x = min(320 - 1, x);

            y = alpha_x * p.x + beta_x * p.y + delta_x;
            // clamp y between 0 and SCREEN_HEIGHT - 1
            y = max(0, y);
            y = min(320 - 1, y);

            
            if(rotation==0){
                // return Location(int((y/320.0)*240),320-int((x/240.0)*320));
                return Location(x,y);
            }else if(rotation==1){
                return Location(y,240-x);
            }else if(rotation==2){
                return Location(240-x,320-y);
            }else if(rotation==3){
                return Location(320-y,x);
            }else{
                return Location(x,y);
            }
            
        }

        void activateTouchscreen(){
            if(!this->useTouchscreen){
                this->mySpiSD->end();
                this->mySpiScreen->begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
                this->ts->begin(*(this->mySpiScreen));
                this->useTouchscreen = true;
            }
        }

        void activateSDcard(){
            if(this->useTouchscreen){
                this->mySpiScreen->end();
                this->mySpiSD->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
                this->useTouchscreen = false;
            }
        }

        

        int getWidth(){return width;}
        int getHeight(){return height;}
        int getRotation(){return rotation;}

        void setRotation(int rot){
            // 0: No rotation: port down
            // 1: Rotate 90 degrees clockwise: port right
            // 2: Rotate 180 degrees: port up
            // 3: Rotate 270 degrees clockwise: 
            this->rotation = rot%4;
            this->ts->setRotation(1);
            this->tft->setRotation(this->rotation);

            if(this->rotation == 0 || this->rotation == 2){
                this->width = 240;
                this->height = 320;
            }else{
                this->width = 320;
                this->height = 240;
            }
        }

        TS_Point getPointTouched(){
            if(useTouchscreen){
                return this->ts->getPoint();
            }else{
                return TS_Point(-1,-1,-1);
            }
        }

        //Load file into stack from FS
        size_t loadFile(uint8_t** jpgBuffer, String fileLocation, fs::FS &fs){
            this->activateSDcard();
            size_t fileSize = 0;

            File file = fs.open(fileLocation);
            if (!file) {
                Serial.println("Failed to open file for reading");
                return fileSize;
            }

            fileSize = file.size();
            Serial.printf("File size: %d\n", fileSize);

            *jpgBuffer = (uint8_t*)malloc((fileSize+1)*sizeof(uint8_t));

            if (*jpgBuffer) {
                for (size_t i = 0; i < fileSize; i++) {
                    (*jpgBuffer)[i] = file.read(); // Read byte by byte into the array
                }
            } else {
                Serial.println("Memory allocation failed!");
            }

            file.close();
            this->activateTouchscreen();
            return fileSize;
        }

    private:
        int rotation;
        bool useTouchscreen;
        int width = 320;
        int height = 240;
        
};


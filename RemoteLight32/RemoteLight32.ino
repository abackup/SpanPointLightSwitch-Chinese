
/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2024 Gregg E. Berman
 *  
 *  https://github.com/HomeSpan/HomeSpan
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
// 这是远程 ESP32 设备的草图。该草图使用 SpanPoint 接收数据并将其传输到主设备（中央集线器）

#include "HomeSpan.h"

#define MAIN_DEVICE_MAC       "AC:67:B2:77:42:20"           // 在中央集线器草图上的串行监视器中输入“i”时，使用 SpanPoint 部分下列出的相应“本地 MAC 地址”
#define LED_PIN               13
#define CALIBRATION_PERIOD    60
#define INTERNAL_ID           32

SpanPoint *mainDevice;
uint32_t calibrateTime=0;

void setup() {
  
  Serial.begin(115200);
  delay(1000);

  Serial.printf("\n\nThis is a REMOTE Device with MAC Address = %s\n",WiFi.macAddress().c_str());
  Serial.printf("NOTE: This MAC Address must be entered into the corresponding SpanPoint() call of the MAIN Device.\n\n");

  mainDevice=new SpanPoint(MAIN_DEVICE_MAC,sizeof(int),sizeof(boolean));

  homeSpan.setLogLevel(1);

  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,0);

}

void loop() {

  if(millis()>calibrateTime){
    int id=INTERNAL_ID;
    boolean success = mainDevice->send(&id);
    Serial.printf("Send %s\n",success?"Succeded":"Failed");
    calibrateTime=millis()+CALIBRATION_PERIOD*1000;
  }

  boolean power;
  if(mainDevice->get(&power)){
    Serial.printf("Received Power=%d\n",power);
    digitalWrite(LED_PIN,power);
  }
}

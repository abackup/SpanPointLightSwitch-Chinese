
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
 
#ifndef ARDUINO_ARCH_ESP8266
#error ERROR: THIS SKETCH IS DESIGNED FOR ESP8266 MICROCONTROLLERS!
#endif

// 这是远程 ESP8266 设备的草图，无法运行 HomeSpan 或 SpanPoint。因此，该草图直接使用 ESP-NOW 函数接收和传输数据到主设备（中央集线器）

uint8_t MAIN_DEVICE_MAC[6]={0xAC,0x67,0xB2,0x77,0x42,0x21};       // 在中央集线器草图上的串行监视器中输入“i”时，使用 SpanPoint 部分下列出的相应“本地 MAC 地址”
#define LED_PIN               0
#define CALIBRATION_PERIOD    20
#define INTERNAL_ID           8266

#include <ESP8266WiFi.h>                 
#include <espnow.h>
#include <Crypto.h>

uint32_t calibrateTime=0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.printf("Last Packet Sent to %02X:%02X:%02X:%02X:%02X:%02X Status: %s\n",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5],sendStatus==0?"Success":"Fail");
}

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  Serial.printf("Packet Receieved from %02X:%02X:%02X:%02X:%02X:%02X: %d bytes\n",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5],len);
  digitalWrite(LED_PIN,!incomingData[0]);     // 我们应该从 Maon 设备接收的唯一字节是一个布尔值，表示 LED 应该打开还是关闭（注意反向 LED：HIGH=OFF，LOW=ON）
}

void setup() {
  
  Serial.begin(115200);
  delay(1000);

  Serial.printf("\n\nThis is a REMOTE Device with MAC Address = %s\n",WiFi.macAddress().c_str());
  Serial.printf("NOTE: This MAC Address must be entered into the corresponding SpanPoint() call of the MAIN Device.\n\n");

  WiFi.mode(WIFI_STA);
  wifi_set_channel(6);              // 在这里，我们需要手动设置通道，以匹配在串行监视器上输入“i”时中央集线器报告的内容

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW.  HALTING!");
    while(1);
  }  

  uint8_t hash[32];                 // 创建空间来存储 32 字节哈希码
  char password[]="HomeSpan";       // 指定密码
  
  experimental::crypto::SHA256::hash(password,strlen(password),hash);     // 创建下面要使用的哈希码

  esp_now_register_send_cb(OnDataSent);                      // 注册发送数据的回调
  esp_now_register_recv_cb(OnDataRecv);                      // 注册接收数据的回调
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);                 // 将此设备的角色设置为控制器（即，它将数据发送到 ESP32）

  esp_now_set_kok(hash+16,16);                                    // 接下来我们设置 PMK。出于某种原因，这在 ESP8266 上称为 KOK。请注意，您必须在添加任何对等点之前设置 PMK
        
  esp_now_add_peer(MAIN_DEVICE_MAC, ESP_NOW_ROLE_COMBO, 0, hash, 16);    // 现在我们添加对等体，设置其角色，并指定 LMK

  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,1);                                   // Adafruit 8266 Featherboard 上的内置 LED 是反向接线的 - HIGH=OFF，LOW=ON

void loop() {

  if(millis()>calibrateTime){
    int id=INTERNAL_ID;
    esp_now_send(MAIN_DEVICE_MAC, (uint8_t *)&id, sizeof(id));     // 将数据发送到主设备！
    calibrateTime=millis()+CALIBRATION_PERIOD*1000;
  }
}

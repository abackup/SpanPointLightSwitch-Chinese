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

// 此示例演示了 HomeSpan 如何通过 SpanPoint 控制远程 ESP32 和远程 ESP8266。
//
// 下面的草图表示连接到 HomeKit 的中央集线器。它与实现两个 LED 控制的教程示例 5 基本相同。但是，此草图不会打开/关闭连接到设备引脚的 LED，
// 而是 HomeSpan 将使用 SpanPoint 向 ESP32 和/或 ESP8266 传输开/关指令。当这两个设备中的任何一个收到开/关指令时，它将相应地打开/关闭 LED。

// 虽然我们只需要将指令从中央集线器传输到两个远程设备，但此草图配置为从远程设备发送和接收数据，以帮助演示该方法。即使您不需要远程设备向中央
// 集线器传输任何内容，定期进行心跳传输也是确保一切正常运转的好方法。此外，如果远程 ESP32 定期将数据传输到中央集线器，SpanPoint 将自动校准 
// WiFi 信道以确保传输成功。

// 由于 SpanPoint 仅在 ESP32 上运行，我们将使用 ESP8266 草图的底层 ESP-NOW 功能。在该草图中，您会注意到没有自动校准，而是需要手动指定
// WiFi 信道以匹配中央集线器连接到家庭网络后使用的任何信道。

#include "HomeSpan.h"

//////////////////////////////////////

struct RemoteLight : Service::LightBulb {

  Characteristic::On power;             // 此特性决定 LED 电源是打开还是关闭
  SpanPoint *remoteDevice;              // 这是用于从远程设备传输/接收数据的 SpanPoint 连接
  
  RemoteLight(const char *macAddress, boolean isESP8266) : Service::LightBulb(){        // 构造函数有两个参数 - 远程设备的 MAC 地址，以及远程设备是否为 ESP8266 的标志

    // 在这里，我们使用远程设备的 MAC 地址创建 SpanPoint 控件：
    // 发送大小设置为布尔值的大小，因为我们将只传输 1 或 0 来请求打开或关闭远程 LED 的电源；
    // 接收大小设置为 int 的大小 - 这将允许我们接受任意“ID”作为来自每个设备的心跳消息的一部分，以密切关注其状态；
    // 队列深度设置为 1；并且
    // 最后一个参数控制 SpanPoint 是否在其 STA MAC 地址（false）或其 AP MAC 地址（true）上侦听传入数据，这是从 ESP8266 接收数据时需要的
    
    remoteDevice=new SpanPoint(macAddress,sizeof(boolean),sizeof(int),1,isESP8266);
  }

  boolean update(){

    boolean lightStatus=power.getNewVal<boolean>();
    
    boolean success = remoteDevice->send(&lightStatus);
    Serial.printf("Send %s\n",success?"Succeded":"Failed");        
   
    return(true);
  }

  void loop(){          // 此循环是可选的——它允许草图扫描来自任一远程设备的任何传入 4 字节（int）消息

    int id;
    if(remoteDevice->get(&id))
      LOG0("Heartbeat received from: %d\n",id);       
  }

};

//////////////////////////////////////
     
void setup() {

  Serial.begin(115200);

  homeSpan.begin(Category::Lighting,"Central Hub");

  new SpanAccessory();  
  new Service::AccessoryInformation();
    new Characteristic::Identify();
  
  new SpanAccessory();  
    new Service::AccessoryInformation(); 
      new Characteristic::Identify();
      new Characteristic::Name("Remote ESP32");               
    new RemoteLight("84:CC:A8:11:B4:84",false);           // 这是运行 SpanPoint 的远程 ESP32 设备的 MAC 地址

  new SpanAccessory();  
    new Service::AccessoryInformation(); 
      new Characteristic::Identify();
      new Characteristic::Name("Remote ESP8266");               
    new RemoteLight("BC:FF:4D:40:8E:71",true);           // 这是运行 ESP-NOW 功能的远程 ESP8266 设备的 MAC 地址（注意第二个参数为 true）
}

//////////////////////////////////////

void loop(){
  
  homeSpan.poll();
  
}

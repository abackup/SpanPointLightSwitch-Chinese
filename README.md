# SpanPointLightSwitch - HomeSpan 项目

SpanPointLightSwitch 是一个工作示例，展示了如何使用 HomeSpan 的 **SpanPoint** 类在连接到 HomeKit 的运行 HomeSpan 的中央集线器和两个未连接到 HomeKit 但直接控制两个 LED 的独立远程设备之间实现双向 ESP-NOW 通信。SpanPointLightSwitch 使用 [HomeSpan](https://github.com/HomeSpan/HomeSpan) HomeKit 库构建，旨在作为 Arduino 草图在 ESP32 设备上运行。

本项目使用的硬件：

* 两个 ESP32 开发板，例如 [Adafruit HUZZAH32 – ESP32 Feather 开发板](https://www.adafruit.com/product/3405)
* 一个 ESP8266 开发板，例如 [Adafruit Feather HUZZAH with ESP8266](https://www.adafruit.com/product/2821)
* 每个开发板内置的 LED

## 概述

本示例包含三个草图：

* [**CentralHub.ino**](CentralHub/CentralHub.ino)：一个完整​​的 HomeSpan 草图，实现了两个 HomeKit LightBulb 配件，但它不是连接连接到设备引脚的两个 LED，而是使用 HomeSpan 的 **SpanPoint** 类将开/关指令传输到远程 ESP32 和远程 ESP8266 设备，每个设备控制自己的 LED

* **RemoteLight32.ino**：一个轻量级的 HomeSpan 草图，不连接完全不与 HomeKit 通信，但仍使用 **SpanPoint** 接收来自中央集线器的消息，请求打开或关闭板载 LED

* **RemoteLight8266.ino**：一个独立的草图，不使用 HomeSpan[^1]，而是直接使用 ESP8286 ESP-NOW 函数接收来自中央集线器的消息，请求打开或关闭板载 LED

## 注释

* 虽然使用连接到 HomeKit 的中央集线器通过 **SpanPoint** 向两个远程设备传达打开/关闭 LED 的指令只需要从中央集线器到远程设备的 *单向* 消息传输，但本例中的草图通过让远程设备也向中央集线器发送定期的“心跳”消息来演示 *双向* SpanPoint/ESP-NOW 通信

* 虽然心跳消息主要用于说明目的，但对于远程 ESP32 设备，它还具有实际好处，即使在该设备上运行的 SpanPoint 实例自动校准用于 ESP-NOW 的 WiFi 信道。这确保两个 ESP32 设备始终处于同一信道，即使中央集线器设备需要更改信道才能连接到中央 WiFi 网络

* 如果您想通过在每个远程设备上添加本地按钮开关来手动打开或关闭连接到该设备的灯来扩展示例草图，也可以使用双向通信。在这样的设置中，每当按下按钮开关时，每个远程设备都会向中央集线器发送开/关状态更新，以便中央集线器可以将状态变化传达回 HomeKit，然后它会正确反映在您的 Home 应用中（有关如何实现按钮开关和将状态变化传达给 HomeKit 的详细示例，请参阅 [HomeSpan 教程](https://github.com/HomeSpan/HomeSpan/blob/master/docs/Tutorials.md) 页面）

* 由于 HomeSpan（以及 SpanPoint）不在 ESP8266 芯片上运行，因此不会执行自动校准，并且 ESP8266 草图已硬编码 WiFi 信道。请确保将其更改为与中央集线器使用的信道匹配[^2]

* 此草图的主要目的是演示如何配置 SpanPoint/ESP-NOW 以促进*中央集线器 ESP32 和远程 ESP8266 之间的双向通信*。如果您使用的只是 ESP32 设备，那么您很可能不会使用中央集线器，只需在每个控制灯的 ESP32 设备上运行 HomeSpan

* 请特别注意每对 SpanPoint/ESP-NOW 连接需要哪些 MAC 地址。对于 ESP32 设备，您可以同时使用 ESP-NOW 并作为站点连接到 WiFi 网络。但是当 ESP8266 将数据传输到 ESP32 时，它必须使用该 ESP32 设备的 Soft-AP MAC 地址。**SpanPoint** 构造函数有一个参数，用于指定 **SpanPoint** 的任何给定时刻是否应使用 STA 或 AP 地址。由于我们的示例包括一个远程 ESP32 和一个远程 ESP8266 设备，因此中央集线器 ESP32 设备的 STA 和 AP MAC 地址均被使用，并且当中央集线器首次启动时，这两个地址都将打印在“SpanPoint 连接”下的串行监视器上

[^1]: HomeSpan（以及 SpanPoint）不在 ESP8266 设备上运行
[^2]: 当然，您可以为 ESP8266 草图开发自己的自动校准逻辑，例如将其*临时*连接到中央 WiFi 网络以识别 WiFi 网络使用的频道

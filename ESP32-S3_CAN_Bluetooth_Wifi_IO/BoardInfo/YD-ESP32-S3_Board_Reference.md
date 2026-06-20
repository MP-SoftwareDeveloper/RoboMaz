# YD-ESP32-S3 Board Reference
## Project: ESP32-S3_CAN_Bluetooth_Wifi_IO

**Manufacturer:** VCC-GND Studio (源地工作室)  
**Module:** ESP32-S3-WROOM-1-N16R8  
**Form factor:** DevKitC-1 compatible, 44-pin (22 per side), 1.1" × 2.5"

---

## Key Specs

| Parameter | Value |
|---|---|
| CPU | Xtensa dual-core LX7, up to 240 MHz |
| Flash | 16 MB (Quad SPI) |
| PSRAM | 8 MB (Octal SPI) |
| SRAM | 512 KB |
| Wi-Fi | 802.11 b/g/n 2.4 GHz |
| Bluetooth | BLE 5.0 |
| USB-to-UART chip | CH343P (WCH Qinheng) — needs CH343 driver |
| Operating voltage | 3.3 V |
| Power input | USB-C 5 V or 5V/3V3 pins |

---

## USB Ports (both USB-C)

| Port | Function |
|---|---|
| UART USB-C (bottom-left) | Flashing, serial monitor — via CH343P |
| OTG USB-C (bottom-right) | ESP32-S3 native USB (GPIO19/20), JTAG debug |

> In Arduino IDE: set **Tools → USB CDC On Boot → Enabled** when using the OTG port.

---

## Onboard Peripherals

| Component | Detail |
|---|---|
| RGB LED | WS2812, driven by **GPIO48** (NOT GPIO38) |
| Power LED | Red, always on when powered — not programmable |
| TX LED | Green, GPIO43 |
| RX LED | Blue, GPIO44 |
| BOOT button | GPIO0 (also user button after boot) |
| RST button | EN/Reset |

> **Critical:** `#define RGB_BUILTIN 48` in Arduino. Use `neopixelWrite()` for color control.

---

## Solder Jumper Pads

| Label | Location | Function |
|---|---|---|
| IN-OUT | Front, left side | Closed = connects USB VBus to 5V pin (bypasses diode). Open = isolated. |
| RGB | Front, right of LED | Capacitor pad for RGB signal — may need closing on some boards |
| USB-OTG | Back | Closed = connects VBus of both USB-C ports together |

> Default on most boards: **all open.**

---

## Pin Map — J1 (Left Side, top→bottom)

| Pin | Name | GPIO | Key Functions |
|---|---|---|---|
| 1 | 3V3 | — | 3.3V power |
| 2 | 3V3 | — | 3.3V power |
| 3 | RST | EN | Reset |
| 4 | IO4 | GPIO4 | TOUCH4, ADC1_CH3 |
| 5 | IO5 | GPIO5 | TOUCH5, ADC1_CH4 |
| 6 | IO6 | GPIO6 | TOUCH6, ADC1_CH5 |
| 7 | IO7 | GPIO7 | TOUCH7, ADC1_CH6 |
| 8 | IO15 | GPIO15 | ADC2_CH4, XTAL_32K_P |
| 9 | IO16 | GPIO16 | ADC2_CH5, XTAL_32K_N |
| 10 | IO17 | GPIO17 | U1TXD, ADC2_CH6 |
| 11 | IO18 | GPIO18 | U1RXD, ADC2_CH7 |
| 12 | IO8 | GPIO8 | TOUCH8, ADC1_CH7, **I2C SDA*** |
| 13 | IO3 | GPIO3 | TOUCH3, ADC1_CH2 |
| 14 | IO46 | GPIO46 | Input only |
| 15 | IO9 | GPIO9 | TOUCH9, ADC1_CH8, **I2C SCL*** |
| 16 | IO10 | GPIO10 | TOUCH10, ADC1_CH9, FSPICS0 |
| 17 | IO11 | GPIO11 | TOUCH11, ADC2_CH0, FSPID |
| 18 | IO12 | GPIO12 | TOUCH12, ADC2_CH1, FSPICLK |
| 19 | IO13 | GPIO13 | TOUCH13, ADC2_CH2, FSPIQ |
| 20 | IO14 | GPIO14 | TOUCH14, ADC2_CH3, FSPIWP |
| 21 | 5V | — | 5V power (see IN-OUT jumper) |
| 22 | GND | — | Ground |

---

## Pin Map — J3 (Right Side, top→bottom)

| Pin | Name | GPIO | Key Functions |
|---|---|---|---|
| 1 | GND | — | Ground |
| 2 | TX | GPIO43 | U0TXD, CLK_OUT1 |
| 3 | RX | GPIO44 | U0RXD, CLK_OUT2 |
| 4 | IO1 | GPIO1 | TOUCH1, ADC1_CH0, I2C SDA (alt) |
| 5 | IO2 | GPIO2 | TOUCH2, ADC1_CH1, I2C SCL (alt) |
| 6 | IO42 | GPIO42 | MTMS |
| 7 | IO41 | GPIO41 | MTDI, CLK_OUT1 |
| 8 | IO40 | GPIO40 | MTDO, CLK_OUT2 |
| 9 | IO39 | GPIO39 | MTCK, CLK_OUT3 |
| 10 | IO38 | GPIO38 | FSPIWP, SUBSPIWP |
| 11 | IO37 | GPIO37 | PSRAM (avoid if N16R8) |
| 12 | IO36 | GPIO36 | PSRAM (avoid if N16R8) |
| 13 | IO35 | GPIO35 | PSRAM (avoid if N16R8) |
| 14 | IO0 | GPIO0 | BOOT button |
| 15 | IO45 | GPIO45 | Strapping pin |
| 16 | IO48 | GPIO48 | **RGB LED (WS2812)** |
| 17 | IO47 | GPIO47 | SPICLK_P |
| 18 | IO21 | GPIO21 | General purpose |
| 19 | IO20 | GPIO20 | **USB D+** (OTG) |
| 20 | IO19 | GPIO19 | **USB D-** (OTG) |
| 21 | GND | — | Ground |
| 22 | GND | — | Ground |

---

## Reserved / Restricted Pins

| GPIO | Reason |
|---|---|
| GPIO35, GPIO36, GPIO37 | Used internally for PSRAM (N16R8 — do NOT use) |
| GPIO19, GPIO20 | USB OTG D-/D+ — avoid if using native USB |
| GPIO43, GPIO44 | Default UART0 TX/RX |
| GPIO46 | Input only, boot strapping |
| GPIO0 | BOOT button / strapping pin |
| GPIO45 | Strapping pin (VDD_SPI level) |
| GPIO48 | RGB LED |

---

## Drivers & Resources

| Item | Link |
|---|---|
| CH343 USB-UART driver (Windows) | http://www.wch-ic.com/products/CH343.html |
| Official VCC-GND GitHub | https://github.com/vcc-gnd/YD-ESP32-S3 |
| VCC-GND data/schematic server | http://124.222.62.86/yd-data/YD-ESP32-S3/ |
| Espressif DevKitC-1 schematic (reference) | https://dl.espressif.com/dl/schematics/SCH_ESP32-S3-DevKitC-1_V1.1_20221130.pdf |
| CircuitPython board definition | https://circuitpython.org/board/yd_esp32_s3_n16r8/ |
| ESP-IDF docs | https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/ |

---

## Notes for CAN / BLE / Wi-Fi Project

- **CAN bus:** ESP32-S3 has no built-in CAN peripheral — use an external **MCP2515** (SPI) or **SN65HVD230** with TWAI (ESP32-S3's built-in TWAI controller on any two GPIOs). TWAI uses GPIO pins freely assignable via IO_MUX.
- **Free GPIOs for project use:** GPIO1–14, GPIO17–18, GPIO21, GPIO38–42, GPIO47 (GPIO35–37 blocked by PSRAM).
- **SPI for MCP2515 suggestion:** GPIO10 (CS), GPIO11 (MOSI), GPIO12 (SCLK), GPIO13 (MISO) — FSPI bus.
- **I2C default:** GPIO8 (SDA), GPIO9 (SCL).
- **Power:** Board has a 1A 5V→3.3V LDO with dedicated wireless LDO — adequate for Wi-Fi+BLE concurrent use.

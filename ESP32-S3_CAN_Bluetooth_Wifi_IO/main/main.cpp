#include "esp_log.h"
#include "esp_rom_sys.h"    // esp_rom_delay_us — ROM function, no driver layer

#include "LED/MAZLed.hpp"

static const char *TAG = "main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "=== YD-ESP32-S3 LED Test (bare metal) ===");

    MAZLed led;
    led.init();

    size_t colourIdx = 0;
    bool   txOn      = false;

    while (true)
    {
        // TX and RX LEDs blink in opposite phase
        txOn = !txOn;
        led.setTxLed(txOn);
        led.setRxLed(!txOn);

        // RGB steps through the colour table
        const MAZLed::Colour &c = MAZLed::kColours[colourIdx];
        led.setRgb(c);

        ESP_LOGI(TAG, "RGB: %-8s (%3u,%3u,%3u)  |  TX: %s  RX: %s",
                 c.name, c.r, c.g, c.b,
                 txOn ? "ON " : "OFF",
                 txOn ? "OFF" : "ON ");

        colourIdx = (colourIdx + 1) % MAZLed::kColourCount;

        esp_rom_delay_us(600000);   // 600 ms — ROM function, no FreeRTOS delay
    }
}

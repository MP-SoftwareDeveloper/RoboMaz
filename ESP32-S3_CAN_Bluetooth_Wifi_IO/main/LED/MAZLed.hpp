#pragma once

#include <cstdint>
#include "led_strip.h"

/**
 * @class MAZLed
 * @brief Controls all onboard LEDs on the YD-ESP32-S3 (N16R8).
 *
 *  LED          GPIO   Colour   Drive
 *  ---------    -----  -------  ---------------------------------------
 *  TX LED       43     Green    Active-LOW bare-metal GPIO output
 *  RX LED       44     Blue     Active-LOW bare-metal GPIO output
 *  RGB LED      48     WS2812   RMT peripheral via led_strip driver
 *
 * TX/RX use direct IO_MUX + GPIO register writes (no driver layer).
 * RGB uses the ESP-IDF led_strip RMT backend for hardware-accurate timing.
 *
 * Prerequisites (sdkconfig.defaults):
 *   CONFIG_ESP_CONSOLE_USB_CDC=y      — frees GPIO43 / GPIO44 from UART0
 *   CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240
 */
class MAZLed
{
public:
    // ── Colour helper ──────────────────────────────────────────────────────
    struct Colour { uint8_t r, g, b; const char *name; };

    static constexpr Colour kColours[] = {
        {255,   0,   0, "Red"    },
        {  0, 255,   0, "Green"  },
        {  0,   0, 255, "Blue"   },
        {255, 255,   0, "Yellow" },
        {  0, 255, 255, "Cyan"   },
        {255,   0, 255, "Magenta"},
        {255, 128,   0, "Orange" },
        {128,   0, 255, "Violet" },
        {255, 255, 255, "White"  },
        {  0,   0,   0, "Off"    },
    };
    static constexpr size_t kColourCount = sizeof(kColours) / sizeof(kColours[0]);

    // ── Lifecycle ──────────────────────────────────────────────────────────
    MAZLed() = default;
    ~MAZLed();

    /** Configure GPIO and RMT. Call once before any other method. */
    void init();

    // ── TX / RX LEDs ───────────────────────────────────────────────────────
    void setTxLed(bool on);  ///< Green LED on GPIO43, active-LOW
    void setRxLed(bool on);  ///< Blue  LED on GPIO44, active-LOW

    // ── RGB LED (WS2812 via RMT) ───────────────────────────────────────────
    void setRgb(uint8_t r, uint8_t g, uint8_t b);
    void setRgb(const Colour &c) { setRgb(c.r, c.g, c.b); }
    void clearRgb();

private:
    // ── Pin assignments ────────────────────────────────────────────────────
    static constexpr uint8_t kTxPin  = 43;
    static constexpr uint8_t kRxPin  = 44;
    static constexpr uint8_t kRgbPin = 48;

    // ── RMT led_strip handle ───────────────────────────────────────────────
    led_strip_handle_t _strip = nullptr;

    // ── Bare-metal GPIO helpers (TX/RX only) ──────────────────────────────
    static void pinModeOutput(uint8_t pin);
    static void pinHigh(uint8_t pin);
    static void pinLow(uint8_t pin);
};

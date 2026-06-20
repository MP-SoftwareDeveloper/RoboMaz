#pragma once

#include <cstdint>

/**
 * @class MAZLed
 * @brief Bare-metal control of all onboard LEDs on the YD-ESP32-S3 (N16R8).
 *
 *  LED          GPIO   Colour   Drive
 *  ---------    -----  -------  --------------------
 *  TX LED       43     Green    Active-LOW GPIO output
 *  RX LED       44     Blue     Active-LOW GPIO output
 *  RGB LED      48     WS2812   Bit-bang via CCOUNT timer
 *
 * NO ESP-IDF driver layer is used.  All hardware access goes directly
 * to IO_MUX and GPIO registers via soc/ struct headers.
 *
 * Prerequisites (sdkconfig.defaults):
 *   CONFIG_ESP_CONSOLE_USB_CDC=y      — frees GPIO43 / GPIO44 from UART0
 *   CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240 — timing constant kCpuMhz must match
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

    /** Configure IO_MUX and GPIO registers. Call once before any other method. */
    void init();

    // ── TX / RX LEDs ───────────────────────────────────────────────────────

    void setTxLed(bool on);  ///< Green LED on GPIO43, active-LOW
    void setRxLed(bool on);  ///< Blue  LED on GPIO44, active-LOW

    // ── RGB LED (WS2812 bit-bang) ──────────────────────────────────────────

    void setRgb(uint8_t r, uint8_t g, uint8_t b);
    void setRgb(const Colour &c) { setRgb(c.r, c.g, c.b); }
    void clearRgb()              { setRgb(0, 0, 0); }

private:
    // ── Pin assignments ────────────────────────────────────────────────────
    static constexpr uint8_t kTxPin  = 43;
    static constexpr uint8_t kRxPin  = 44;
    static constexpr uint8_t kRgbPin = 48;

    // ── WS2812B timing ─────────────────────────────────────────────────────
    // Timing is expressed in CPU cycles. kCpuMhz MUST match sdkconfig.
    // Modify only this constant if you change the CPU frequency.
    static constexpr uint32_t kCpuMhz = 240;

    static constexpr uint32_t kT0H = (400u  * kCpuMhz) / 1000u;   //  400 ns high for '0'
    static constexpr uint32_t kT0L = (850u  * kCpuMhz) / 1000u;   //  850 ns low  for '0'
    static constexpr uint32_t kT1H = (800u  * kCpuMhz) / 1000u;   //  800 ns high for '1'
    static constexpr uint32_t kT1L = (450u  * kCpuMhz) / 1000u;   //  450 ns low  for '1'
    static constexpr uint32_t kRst = (50000u * kCpuMhz) / 1000u;  //   50 µs reset pulse

    // ── Register-level GPIO helpers ────────────────────────────────────────
    static void pinModeOutput(uint8_t pin); ///< IO_MUX select + GPIO enable
    static void pinHigh(uint8_t pin);       ///< GPIO out_w1ts / out1_w1ts
    static void pinLow(uint8_t pin);        ///< GPIO out_w1tc / out1_w1tc

    // ── Xtensa cycle counter ───────────────────────────────────────────────
    static uint32_t getCycles();            ///< Read CCOUNT special register
    static void     waitCycles(uint32_t n); ///< Busy-wait for n cycles

    // ── WS2812B protocol ───────────────────────────────────────────────────
    static void ws2812Bit(bool one);        ///< Emit one WS2812 data bit
    static void ws2812Byte(uint8_t byte);   ///< Emit 8 bits MSB-first
    static void ws2812Reset();              ///< >50 µs low → latch pixel data
};

#include "MAZLed.hpp"

// ── Bare-metal register headers ───────────────────────────────────────────────
//
//  soc/gpio_struct.h   — GPIO device struct (GPIO.out_w1ts, .enable1_w1ts, ...)
//
//  No gpio_driver.h, no led_strip.h — direct hardware access only.
//
#include "soc/gpio_struct.h"    // extern gpio_dev_t GPIO
#include "freertos/portmacro.h" // portENTER_CRITICAL / portEXIT_CRITICAL
                                // (spinlock disables interrupts on BOTH cores,
                                //  essential on dual-core ESP32-S3 to prevent
                                //  cache-miss glitches mid-WS2812 stream)

// ── Critical-section lock ─────────────────────────────────────────────────────
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

// =============================================================================
// IO_MUX + GPIO register helpers
// =============================================================================

/**
 * Returns a pointer to the IO_MUX pad-configuration register for a given GPIO.
 *
 * ESP32-S3 IO_MUX register map (TRM §7):
 *   Base address : 0x60009000
 *   Pad N        : base + 4 + N * 4
 *
 * Relevant fields inside the 32-bit register:
 *   [12:10]  MCU_SEL   — 1 = route through GPIO Matrix (standard GPIO)
 *   [ 9: 8]  FUN_DRV   — drive strength (0=5mA … 3=40mA)
 *   [     7] FUN_IE    — input enable
 *   [     6] FUN_WPU   — pull-up enable
 *   [     5] FUN_WPD   — pull-down enable
 */
static inline volatile uint32_t *iomuxReg(uint8_t pin)
{
    return reinterpret_cast<volatile uint32_t *>(0x60009000UL + 4UL + static_cast<uint32_t>(pin) * 4UL);
}

/**
 * Configure a GPIO pin as a push-pull output using direct register writes.
 *
 * Steps:
 *  1. IO_MUX  — MCU_SEL = 1  (GPIO Matrix path, clears other function bits)
 *  2. GPIO Matrix — func_out_sel = 256 (SIG_GPIO_OUT_IDX: use GPIO.out register)
 *                   oen_sel      = 0   (OEN from GPIO.enable, not from peripheral)
 *  3. GPIO enable — set output-enable bit in GPIO.enable_w1ts or .enable1_w1ts
 */
void MAZLed::pinModeOutput(uint8_t pin)
{
    // 1. IO_MUX: set MCU_SEL = 1, leave drive strength at default (bits [9:8] = 0b10 → 20 mA)
    volatile uint32_t *mux = iomuxReg(pin);
    *mux = (*mux & ~(0x7u << 10)) | (1u << 12);  // clear [12:10], set bit 12

    // 2. GPIO Matrix output selection
    //    func_sel = 256 (0x100) = SIG_GPIO_OUT_IDX → data from GPIO.out register
    GPIO.func_out_sel_cfg[pin].func_sel = 256;
    GPIO.func_out_sel_cfg[pin].oen_sel  = 0;   // output-enable from GPIO.enable

    // 3. Enable output driver
    if (pin < 32) {
        GPIO.enable_w1ts = (1u << pin);
    } else {
        GPIO.enable1_w1ts.val = (1u << (pin - 32u));
    }
}

/** Set a GPIO pin HIGH (write-1-to-set register — atomic, no read-modify-write). */
void MAZLed::pinHigh(uint8_t pin)
{
    if (pin < 32) {
        GPIO.out_w1ts = (1u << pin);
    } else {
        GPIO.out1_w1ts.val = (1u << (pin - 32u));
    }
}

/** Set a GPIO pin LOW (write-1-to-clear register — atomic). */
void MAZLed::pinLow(uint8_t pin)
{
    if (pin < 32) {
        GPIO.out_w1tc = (1u << pin);
    } else {
        GPIO.out1_w1tc.val = (1u << (pin - 32u));
    }
}

// =============================================================================
// Xtensa CCOUNT — cycle-accurate busy-wait
// =============================================================================

/**
 * Read the Xtensa CCOUNT special register.
 * CCOUNT increments every CPU clock cycle and wraps at 2^32.
 * At 240 MHz one cycle ≈ 4.17 ns.
 */
uint32_t MAZLed::getCycles()
{
    uint32_t ccount;
    __asm__ volatile("rsr.ccount %0" : "=r"(ccount));
    return ccount;
}

/**
 * Busy-wait for exactly n CPU cycles.
 * Subtraction wraps naturally on uint32_t, so CCOUNT roll-over is handled.
 */
void MAZLed::waitCycles(uint32_t n)
{
    const uint32_t start = getCycles();
    while ((getCycles() - start) < n) { /* spin */ }
}

// =============================================================================
// WS2812B bit-bang protocol
//
//  Colour order : GRB (green first)
//  Bit order    : MSB first
//  Timing       : see kT0H / kT0L / kT1H / kT1L in MAZLed.hpp
//
//  MUST be called inside portENTER_CRITICAL to prevent interrupt-induced
//  timing jitter from corrupting the pulse widths.
// =============================================================================

void MAZLed::ws2812Bit(bool one)
{
    const uint32_t tHigh = one ? kT1H : kT0H;
    const uint32_t tLow  = one ? kT1L : kT0L;

    pinHigh(kRgbPin);
    waitCycles(tHigh);
    pinLow(kRgbPin);
    waitCycles(tLow);
}

void MAZLed::ws2812Byte(uint8_t byte)
{
    for (int8_t bit = 7; bit >= 0; --bit) {
        ws2812Bit(static_cast<bool>((byte >> bit) & 1u));
    }
}

/** Hold the data line LOW for > 50 µs to latch the pixel data into the LED. */
void MAZLed::ws2812Reset()
{
    pinLow(kRgbPin);
    waitCycles(kRst);
}

// =============================================================================
// Public API
// =============================================================================

void MAZLed::init()
{
    pinModeOutput(kTxPin);
    pinModeOutput(kRxPin);
    pinModeOutput(kRgbPin);

    pinHigh(kTxPin);   // TX LED off  (active-LOW: HIGH = off)
    pinHigh(kRxPin);   // RX LED off
    ws2812Reset();     // RGB idle (line low, pixel latched to off)
}

void MAZLed::setTxLed(bool on)
{
    on ? pinLow(kTxPin) : pinHigh(kTxPin);   // active-LOW
}

void MAZLed::setRxLed(bool on)
{
    on ? pinLow(kRxPin) : pinHigh(kRxPin);   // active-LOW
}

/**
 * Drive the WS2812 RGB LED to the requested colour.
 *
 * The full 24-bit stream (GRB, 3 × 8 bits) is transmitted inside a
 * critical section so interrupts cannot stretch any bit-period.
 * The reset pulse follows outside the critical section.
 */
void MAZLed::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    portENTER_CRITICAL(&s_mux);
    ws2812Byte(g);   // WS2812 expects Green first
    ws2812Byte(r);
    ws2812Byte(b);
    portEXIT_CRITICAL(&s_mux);

    ws2812Reset();   // latch — timing non-critical here
}

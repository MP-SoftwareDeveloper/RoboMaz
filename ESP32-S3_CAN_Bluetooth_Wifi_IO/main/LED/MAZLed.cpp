#include "MAZLed.hpp"

#include "soc/gpio_struct.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "MAZLed";

// =============================================================================
// IO_MUX + GPIO bare-metal helpers  (TX / RX LEDs only)
// =============================================================================

static inline volatile uint32_t *iomuxReg(uint8_t pin)
{
    return reinterpret_cast<volatile uint32_t *>(0x60009000UL + 4UL + static_cast<uint32_t>(pin) * 4UL);
}

void MAZLed::pinModeOutput(uint8_t pin)
{
    volatile uint32_t *mux = iomuxReg(pin);
    *mux = (*mux & ~(0x7u << 10)) | (1u << 12);

    GPIO.func_out_sel_cfg[pin].func_sel = 256;
    GPIO.func_out_sel_cfg[pin].oen_sel  = 0;

    if (pin < 32) {
        GPIO.enable_w1ts = (1u << pin);
    } else {
        GPIO.enable1_w1ts.val = (1u << (pin - 32u));
    }
}

void MAZLed::pinHigh(uint8_t pin)
{
    if (pin < 32) {
        GPIO.out_w1ts = (1u << pin);
    } else {
        GPIO.out1_w1ts.val = (1u << (pin - 32u));
    }
}

void MAZLed::pinLow(uint8_t pin)
{
    if (pin < 32) {
        GPIO.out_w1tc = (1u << pin);
    } else {
        GPIO.out1_w1tc.val = (1u << (pin - 32u));
    }
}

// =============================================================================
// Lifecycle
// =============================================================================

void MAZLed::init()
{
    // TX / RX LEDs — bare-metal GPIO output, active-LOW
    pinModeOutput(kTxPin);
    pinModeOutput(kRxPin);
    pinHigh(kTxPin);   // off
    pinHigh(kRxPin);   // off

    // RGB LED — WS2812B on GPIO48, driven via RMT peripheral
    led_strip_config_t strip_cfg = {};
    strip_cfg.strip_gpio_num         = kRgbPin;
    strip_cfg.max_leds               = 1;
    strip_cfg.led_model              = LED_MODEL_WS2812;
    strip_cfg.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;

    led_strip_rmt_config_t rmt_cfg = {};
    rmt_cfg.resolution_hz = 10 * 1000 * 1000;  // 10 MHz — 100 ns resolution

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &_strip));
    led_strip_clear(_strip);
}

MAZLed::~MAZLed()
{
    if (_strip) {
        led_strip_clear(_strip);
        led_strip_del(_strip);
    }
}

// =============================================================================
// TX / RX LEDs
// =============================================================================

void MAZLed::setTxLed(bool on)
{
    on ? pinLow(kTxPin) : pinHigh(kTxPin);
}

void MAZLed::setRxLed(bool on)
{
    on ? pinLow(kRxPin) : pinHigh(kRxPin);
}

// =============================================================================
// RGB LED
// =============================================================================

void MAZLed::setRgb(uint8_t r, uint8_t g, uint8_t b)
{
    if (!_strip) return;
    ESP_ERROR_CHECK(led_strip_set_pixel(_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(_strip));
}

void MAZLed::clearRgb()
{
    if (_strip) led_strip_clear(_strip);
}

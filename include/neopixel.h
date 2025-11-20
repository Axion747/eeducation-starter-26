#ifndef __NEOPIXEL_H__
#define __NEOPIXEL_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "pin.h"

static constexpr uint32_t NEOPIXEL_DEFAULT_INTERVAL_MS = 200;

typedef struct neopixel {
    Adafruit_NeoPixel* strip;
    pin_t pin;
    uint16_t count;
    uint32_t interval_ms;
    uint32_t last_update_ms;
    uint16_t active_pixel;
    uint8_t color_index;
} neopixel_t;

bool neopixel_init(neopixel_t* neo, pin_t pin, uint16_t count);
void neopixel_process(neopixel_t* neo);
void neopixel_set_interval(neopixel_t* neo, uint32_t interval_ms);
void neopixel_shutdown(neopixel_t* neo);
void neopixel_get_state(const neopixel_t* neo,
                        uint16_t* active_pixel,
                        uint8_t* color_index,
                        uint32_t* interval_ms,
                        bool* initialized);

#endif // __NEOPIXEL_H__

#include "neopixel.h"

#include <new>

namespace {
struct rgb_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static const rgb_color kRgbSequence[] = {
    {255, 0, 0},  // Red
    {0, 255, 0},  // Green
    {0, 0, 255}   // Blue
};
static constexpr uint8_t kColorCount = sizeof(kRgbSequence) / sizeof(kRgbSequence[0]);
}

bool neopixel_init(neopixel_t* neo, pin_t pin, uint16_t count) {
    if (neo == nullptr || count == 0) {
        return false;
    }

    neopixel_shutdown(neo);

    neo->pin = pin;
    neo->count = count;
    neo->interval_ms = NEOPIXEL_DEFAULT_INTERVAL_MS;
    neo->last_update_ms = 0;
    neo->active_pixel = 0;
    neo->color_index = 0;

    neo->strip = new (std::nothrow) Adafruit_NeoPixel(count, static_cast<int16_t>(pin), NEO_GRB + NEO_KHZ800);
    if (neo->strip == nullptr) {
        return false;
    }

    neo->strip->begin();
    neo->strip->clear();
    neo->strip->show();

    return true;
}

void neopixel_set_interval(neopixel_t* neo, uint32_t interval_ms) {
    if (neo == nullptr) {
        return;
    }
    if (interval_ms == 0) {
        interval_ms = 1;
    }
    neo->interval_ms = interval_ms;
}

void neopixel_process(neopixel_t* neo) {
    if (neo == nullptr || neo->strip == nullptr) {
        return;
    }

    uint32_t now = millis();
    if (now - neo->last_update_ms < neo->interval_ms) {
        return;
    }
    neo->last_update_ms = now;

    const rgb_color& color = kRgbSequence[neo->color_index];
    neo->strip->clear();
    neo->strip->setPixelColor(neo->active_pixel, color.r, color.g, color.b);
    neo->strip->show();

    neo->active_pixel = (neo->active_pixel + 1) % neo->count;
    if (neo->active_pixel == 0) {
        neo->color_index = (neo->color_index + 1) % kColorCount;
    }
}

void neopixel_shutdown(neopixel_t* neo) {
    if (neo == nullptr) {
        return;
    }
    if (neo->strip != nullptr) {
        neo->strip->clear();
        neo->strip->show();
        delete neo->strip;
        neo->strip = nullptr;
    }
}

void neopixel_get_state(const neopixel_t* neo,
                        uint16_t* active_pixel,
                        uint8_t* color_index,
                        uint32_t* interval_ms,
                        bool* initialized) {
    if (initialized != nullptr) {
        bool ready = (neo != nullptr && neo->strip != nullptr);
        *initialized = ready;
    }
    if (neo == nullptr) {
        return;
    }
    if (active_pixel != nullptr) {
        *active_pixel = neo->active_pixel;
    }
    if (color_index != nullptr) {
        *color_index = neo->color_index;
    }
    if (interval_ms != nullptr) {
        *interval_ms = neo->interval_ms;
    }
}

#include "button.h"

void button_init(button_t* btn, pin_t pin) {
    btn->pin = pin;
    btn->callback = NULL;
    btn->ctx = NULL;
    btn->event_pending = false;
    btn->last_raw = digitalRead(pin);
    btn->last_debounce_ms = 0;
    btn->debounce_ms = 20; // default debounce 20 ms
    btn->stable_state = (btn->last_raw == HIGH); // assume pressed if LOW

    pinMode(pin, INPUT_PULLUP);

    attach_button_interrupt(btn, pin);
}

void button_set_callback(button_t* btn, void (*cb)(button_t* ctx), void* ctx) {
    btn->callback = cb;
    btn->ctx = ctx;
}

bool button_read(const button_t* btn) {
    return btn->stable_state;
}

static void __button_callback(void *ctx) {
    button_t *btn = (button_t *)(ctx);

    if (!btn) return;
    btn->last_raw = digitalRead(btn->pin);
    btn->event_pending = true;
}

void attach_button_interrupt(button_t *btn, pin_t pin) {
    attachInterruptArg(digitalPinToInterrupt(pin), __button_callback, btn, CHANGE);
}

void button_process(button_t* btn) {
    // Should be called from loop() or a task
    if (!btn) return;

    if (!btn->event_pending) return;

    // Start debounce timer
    uint32_t now = millis();
    if (btn->last_debounce_ms == 0) {
        btn->last_debounce_ms = now;
        return;
    }

    // Wait until stable interval has passed
    if ((now - btn->last_debounce_ms) < btn->debounce_ms) return;

    // debounce period passed — accept sampled level
    btn->last_debounce_ms = 0;
    btn->event_pending = false;

    bool pressed = (btn->last_raw == HIGH);
    if (pressed != btn->stable_state) {
        btn->stable_state = pressed;
        if (pressed && btn->callback) {
            btn->callback(btn);
        }
    }
}

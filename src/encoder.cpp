#include "encoder.h"

static void __encoder_isr_a(void* ctx) {
    encoder_t* enc = (encoder_t*)ctx;

    uint8_t a = digitalRead(enc->pin_a);
    uint8_t b = digitalRead(enc->pin_b);
    uint8_t curr = (a << 1) | b;

    uint8_t idx = (enc->last_state << 2) | curr;
    static const int8_t tbl[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    int8_t delta = tbl[idx];
    if (delta) {
        enc->position += delta;
        enc->last_state = curr;
        if (enc->spin_cb) enc->spin_cb(enc, delta);
    } else {
        enc->last_state = curr;
    }
}

static void __encoder_isr_b(void* ctx) {
    encoder_t* enc = (encoder_t*)ctx;

    uint8_t a = digitalRead(enc->pin_a);
    uint8_t b = digitalRead(enc->pin_b);
    uint8_t curr = (a << 1) | b;

    uint8_t idx = (enc->last_state << 2) | curr;
    static const int8_t tbl[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    int8_t delta = tbl[idx];
    if (delta) {
        enc->position += delta;
        enc->last_state = curr;
        if (enc->spin_cb) enc->spin_cb(enc, delta);
    } else {
        enc->last_state = curr;
    }
}

static void __encoder_isr_btn(void* ctx) {
    encoder_t* enc = (encoder_t*)ctx;
    // how to call the callback
    if (enc->button_cb) enc->button_cb(enc);
}

void encoder_init(encoder_t* enc, pin_t pin_a, pin_t pin_b, pin_t pin_btn) {
    enc->pin_a = pin_a;
    enc->pin_b = pin_b;
    enc->pin_btn = pin_btn;
    enc->position = 0;
    enc->last_state = 0;
    enc->spin_cb = NULL;
    enc->button_cb = NULL;

    pinMode(enc->pin_a, INPUT_PULLUP);
    pinMode(enc->pin_b, INPUT_PULLUP);
    pinMode(enc->pin_btn, INPUT_PULLUP);

    // read initial state
    uint8_t a = digitalRead(enc->pin_a);
    uint8_t b = digitalRead(enc->pin_b);
    enc->last_state = (a << 1) | b;

    attach_encoder_interrupts(enc);
}

void encoder_set_spin_callback(encoder_t* enc, void (*cb)(encoder_t* enc, int32_t delta)) {
    enc->spin_cb = cb;
}

void encoder_set_button_callback(encoder_t* enc, void (*cb)(encoder_t* enc)) {
    enc->button_cb = cb;
}

int32_t encoder_get_position(const encoder_t* enc) {
    return enc->position;
}

void encoder_set_position(encoder_t* enc, int32_t pos) {
    enc->position = pos;
}

void attach_encoder_interrupts(encoder_t* enc) {
    attachInterruptArg(digitalPinToInterrupt(enc->pin_a), __encoder_isr_a, enc, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(enc->pin_b), __encoder_isr_b, enc, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(enc->pin_btn), __encoder_isr_btn, enc, RISING);
}
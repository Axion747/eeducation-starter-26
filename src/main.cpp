#include <Arduino.h>
#include <button.h>
#include <encoder.h>

button_t button;
encoder_t encoder;

static void on_enc_push_button(button_t *btn) {
}

static void on_push_button(button_t *btn) {
}

void setup() {
    Serial.begin(9600);

    button_init(&button, BTN_0);
    button_set_callback(&button, on_push_button, NULL);

    encoder_init(&encoder, RE_CW, RE_CCW, RE_BTN);
}

void loop() {
    static int32_t last_pos = 0;
    int32_t pos = encoder_get_position(&encoder);
    if (pos != last_pos) {
        Serial.printf("Encoder position: %ld\n", pos);
        last_pos = pos;
    }
    button_process(&button);

    bool down = button_read(&button);
    static uint32_t t = 0;
    if (millis() - t > 500) {
        Serial.printf("Button status : %d\n", down);
        t = millis();
    }

    delay(5);
}
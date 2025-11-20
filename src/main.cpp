#include <Arduino.h>
#include <button.h>
#include <encoder.h>
#include <imu.h>
#include <neopixel.h>
#include <BleKeyboard.h>

static constexpr int32_t kEncoderDeadband = 1;
static constexpr uint32_t kGateToggleDebounceMs = 750;
static constexpr uint32_t kButtonRepeatIntervalMs = 80;

button_t button;
encoder_t encoder;
imu_t imu;
neopixel_t neopixel = {};
BleKeyboard bleKeyboard("EEducation Keyboard", "Benson and Sabil", 100);
static bool keyboard_gate_active = false;
static bool keyboard_gate_last_state = false;
static uint32_t last_gate_toggle_ms = 0;
static bool encoder_button_was_pressed = false;
static uint32_t last_button_a_emit_ms = 0;
static bool w_hold_active = false;
static bool s_hold_active = false;

static const char* neopixel_color_name(uint8_t color_index) {
    switch (color_index) {
        case 0:
            return "Red";
        case 1:
            return "Green";
        case 2:
            return "Blue";
        default:
            return "Unknown";
    }
}

static void send_key_press(char key_code) {
    Serial.write(static_cast<uint8_t>(key_code));
    if (bleKeyboard.isConnected()) {
        bleKeyboard.write(static_cast<uint8_t>(key_code));
    }
}

static void apply_key_hold(char key_code, bool desired_state, bool& cached_state) {
    if (cached_state == desired_state) {
        return;
    }
    cached_state = desired_state;
    if (bleKeyboard.isConnected()) {
        if (desired_state) {
            bleKeyboard.press(static_cast<uint8_t>(key_code));
        } else {
            bleKeyboard.release(static_cast<uint8_t>(key_code));
        }
    }
    Serial.printf("%c %s\n", key_code, desired_state ? "DOWN" : "UP");
}

void setup() {
    Serial.begin(115200);
    bleKeyboard.begin();

    button_init(&button, BTN_1);
    button_set_callback(&button, nullptr, NULL);

    encoder_init(&encoder, RE_CW, RE_CCW, RE_BTN);

    if (!imu_init(&imu, IMU_INT, 0x68, &Wire)) { // gonna be so honest, idk how the wire shit works; gonna pray it does
        Serial.println("IMU initialization failed!"); // if this shows, we fucked.
    }

    if (!neopixel_init(&neopixel, NEO_DATA, 3)) {
        Serial.println("NeoPixel init failed");
    } else {
        neopixel_set_interval(&neopixel, 150);
    }
}

void loop() {
    button_process(&button);

    uint32_t now = millis();
    bool encoder_button_pressed = (digitalRead(static_cast<int>(encoder.pin_btn)) == LOW);
    if (encoder_button_pressed && !encoder_button_was_pressed && (now - last_gate_toggle_ms) >= kGateToggleDebounceMs) {
        bool next_state = !keyboard_gate_last_state;
        keyboard_gate_active = next_state;
        keyboard_gate_last_state = next_state;
        encoder_set_position(&encoder, 0);
        last_gate_toggle_ms = now;
        apply_key_hold('W', false, w_hold_active);
        apply_key_hold('S', false, s_hold_active);
        Serial.printf("Keyboard gate %s\n", keyboard_gate_active ? "ENABLED" : "DISABLED");
    }
    encoder_button_was_pressed = encoder_button_pressed;

    int32_t encoder_pos = encoder_get_position(&encoder);
    bool want_w = keyboard_gate_active && (encoder_pos >= kEncoderDeadband);
    bool want_s = keyboard_gate_active && (encoder_pos <= -kEncoderDeadband);
    if (want_w && want_s) {
        want_w = false;
        want_s = false;
    }
    apply_key_hold('W', want_w, w_hold_active);
    apply_key_hold('S', want_s, s_hold_active);

    bool action_button_down = button_read(&button);
    if (action_button_down) {
        if ((now - last_button_a_emit_ms) >= kButtonRepeatIntervalMs) {
            send_key_press('D');
            last_button_a_emit_ms = now;
        }
    } else {
        last_button_a_emit_ms = now;
    }

    static uint32_t last_print = 0;
    if (millis() - last_print > 1000) { // 500 ms intervals
        int32_t pos = encoder_pos;
        bool buttonStatus = action_button_down;
        // imu_data_t data;
        // uint16_t neo_pixel = 0;
        // uint8_t neo_color_index = 0;
        // uint32_t neo_interval_ms = 0;
        // bool neo_ready = false;
        // neopixel_get_state(&neopixel, &neo_pixel, &neo_color_index, &neo_interval_ms, &neo_ready);
        // const char* neo_color_name = neopixel_color_name(neo_color_index);
        bool ble_connected = bleKeyboard.isConnected();

        // if (imu_read(&imu, &data)) {
            Serial.printf(
                "Enc:%ld | Btn:%d | KeyGate:%d | BLE:%s\n",
                pos, buttonStatus,
                // "Enc:%ld | Btn:%d | KeyGate:%d | Accel: X=%.2fg Y=%.2fg Z=%.2fg | Gyro: X=%.2f°/s Y=%.2f°/s Z=%.2f°/s | Temp:%.1f°C | Neo:%s Pix:%u Color:%s(%u) Interval:%lums | BLE:%s\n",
                // pos, down,
                keyboard_gate_active,
                // data.accel_x, data.accel_y, data.accel_z,
                // data.gyro_x, data.gyro_y, data.gyro_z,
                // data.temp,
                // neo_ready ? "OK" : "ERR",
                // neo_ready ? neo_pixel : 0,
                // neo_color_name,
                // neo_color_index,
                // static_cast<unsigned long>(neo_interval_ms),
                ble_connected ? "CONNECTED" : "OFFLINE");
        // }
        last_print = millis();
    }

    neopixel_process(&neopixel);

    delay(5);
}
#include <Arduino.h>
#include <button.h>
#include <encoder.h>
#include <imu.h>

button_t button;
encoder_t encoder;
imu_t imu;

static void on_enc_push_button(button_t *btn) {
}

static void on_push_button(button_t *btn) {
}

void setup() {
    Serial.begin(9600);

    button_init(&button, BTN_0);
    button_set_callback(&button, on_push_button, NULL);

    encoder_init(&encoder, RE_CW, RE_CCW, RE_BTN);

    if (!imu_init(&imu, IMU_INT, 0x68, &Wire)) { // gonna be so honest, idk how the wire shit works; gonna pray it does
        Serial.println("IMU initialization failed!"); // if this shows, we fucked.
    }
}

void loop() {
    button_process(&button);

    static uint32_t last_print = 0;
    if (millis() - last_print > 500) { // 500 ms intervals
        int32_t pos = encoder_get_position(&encoder);
        bool down = button_read(&button);
        imu_data_t data;
        
        if (imu_read(&imu, &data)) {
            Serial.printf("Enc:%ld | Btn:%d | Accel: X=%.2fg Y=%.2fg Z=%.2fg | Gyro: X=%.2f°/s Y=%.2f°/s Z=%.2f°/s | Temp:%.1f°C\n",
                pos, down,
                data.accel_x, data.accel_y, data.accel_z,
                data.gyro_x, data.gyro_y, data.gyro_z,
                data.temp);
        }
        last_print = millis();
    }

    delay(5);
}
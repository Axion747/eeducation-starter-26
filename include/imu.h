#ifndef __IMU_H__
#define __IMU_H__

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include "pin.h"

/**
 * @brief Simple IMU (Inertial Measurement Unit) abstraction for BMI323.
 * 
 * This structure and associated functions provide a basic interface
 * for initializing and reading data from a BMI323 IMU sensor via I2C.
 */

typedef struct imu_data {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temp;
} imu_data_t;

typedef struct imu {
    uint8_t i2c_addr;
    pin_t int_pin;
    TwoWire* wire;
    bool initialized;
} imu_t;

/**
 * @brief Initializes the IMU
 * 
 * @param imu Pointer to imu instance
 * @param int_pin Interrupt pin (GPIO_NUM_27 by default)
 * @param i2c_addr I2C address (0x68 or 0x69)
 * @param wire Pointer to TwoWire instance
 * @return true if initialization successful, false otherwise
 */
bool imu_init(imu_t* imu, pin_t int_pin, uint8_t i2c_addr, TwoWire* wire);

/**
 * @brief Reads acceleration and gyroscope data from IMU.
 * 
 * @param imu Pointer to imu instance
 * @param data Pointer to imu_data structure to store results
 * @return true if read successful, false otherwise
 */
bool imu_read(imu_t* imu, imu_data_t* data);

/**
 * @brief Reads temperature from IMU.
 * 
 * @param imu Pointer to imu instance
 * @param temp Pointer to store temperature value in °C
 * @return true if read successful, false otherwise
 */
bool imu_read_temp(imu_t* imu, float* temp);

/**
 * @brief Reads raw acceleration data from IMU.
 * 
 * @param imu Pointer to imu instance
 * @param x Pointer to store X-axis value (g)
 * @param y Pointer to store Y-axis value (g)
 * @param z Pointer to store Z-axis value (g)
 * @return true if read successful, false otherwise
 */
bool imu_read_accel(imu_t* imu, float* x, float* y, float* z);

/**
 * @brief Reads raw gyroscope data from IMU.
 * 
 * @param imu Pointer to imu instance
 * @param x Pointer to store X-axis value (deg/s)
 * @param y Pointer to store Y-axis value (deg/s)
 * @param z Pointer to store Z-axis value (deg/s)
 * @return true if read successful, false otherwise
 */
bool imu_read_gyro(imu_t* imu, float* x, float* y, float* z);

#endif // __IMU_H__
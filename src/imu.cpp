#include "imu.h"

// I2C address and register definitions
#define BMI323_I2C_ADDR         0x68  // Default I2C address (SDO = GND)
#define CHIP_ID_REG             0x00
#define ERR_REG                 0x01
#define STATUS_REG              0x02
#define ACC_CONF_REG            0x20
#define GYR_CONF_REG            0x21
#define ACC_DATA_X_REG          0x03
#define ACC_DATA_Y_REG          0x04
#define ACC_DATA_Z_REG          0x05
#define GYR_DATA_X_REG          0x06
#define GYR_DATA_Y_REG          0x07
#define GYR_DATA_Z_REG          0x08
#define TEMP_DATA_REG           0x09
#define CMD_REG                 0x7E
#define FEATURE_IO0_REG         0x10
#define FEATURE_IO1_REG         0x11
#define FEATURE_IO2_REG         0x12
#define FEATURE_IO_STATUS_REG   0x14
#define FEATURE_CTRL_REG        0x40

// Configuration constants
#define BMI323_CHIP_ID              0x00
#define OUTPUT_RATE_HZ              20     // 20Hz data output
#define ACC_ANGLE_LSB_PER_G         4096.0    // deg/s range
#define GYRO_ANGLE_LSB_PER_DPS      16.384  // ~2000 deg/s range
#define SOFT_RESET_CMD              0xDEAF    // soft reset command
#define ACC_CONF_NORMAL_100HZ_8G    0x4028    // accel: 100Hz, +- 8g
#define GYR_CONF_NORMAL_100HZ_2000DPS 0x4048  // gyro: 100Hz, +-2000 deg/s

// Scale factors
static float accel_scale = 1.0f / ACC_ANGLE_LSB_PER_G;
static float gyro_scale = 1.0f / GYRO_ANGLE_LSB_PER_DPS;

static void writeRegister16(imu_t* imu, uint8_t reg, uint16_t data) {
    imu->wire->beginTransmission(imu->i2c_addr);
    imu->wire->write(reg);
    imu->wire->write((uint8_t)(data & 0xFF));        // LSB
    imu->wire->write((uint8_t)((data >> 8) & 0xFF)); // MSB
    imu->wire->endTransmission();
}

// Helper function to read 16-bit register
static uint16_t readRegister16(imu_t* imu, uint8_t reg) {
    imu->wire->beginTransmission(imu->i2c_addr);
    imu->wire->write(reg);
    if (imu->wire->endTransmission(false) != 0) {
        return 0;
    }
    
    if (imu->wire->requestFrom(imu->i2c_addr, (uint8_t)2) != 2) {
        return 0;
    }
    
    uint8_t lsb = imu->wire->read();
    uint8_t msb = imu->wire->read();
    return (uint16_t)((msb << 8) | lsb);
}

// Convert raw accelerometer data to g
static float convertAccelData(uint16_t rawData) {
    int16_t signedData = (int16_t)rawData;
    return signedData * accel_scale;
}

// Convert raw gyroscope data to deg/s
static float convertGyroData(uint16_t rawData) {
    int16_t signedData = (int16_t)rawData;
    return signedData * gyro_scale;
}

// Convert raw temperature data to °C
static float convertTempData(uint16_t rawData) {
    int16_t signedData = (int16_t)rawData;
    return (signedData / 512.0f) + 23.0f;
}

// Initialize feature engine
static bool initializeFeatureEngine(imu_t* imu) {
    // Enable feature engine if needed
    writeRegister16(imu, FEATURE_CTRL_REG, 0x0001);
    delay(10);
    return true;
}

bool imu_init(imu_t* imu, pin_t int_pin, uint8_t i2c_addr, TwoWire* wire) {
    if (!imu || !wire) {
        return false;
    }
    
    imu->i2c_addr = i2c_addr;
    imu->int_pin = int_pin;
    imu->wire = wire;
    imu->initialized = false;
    
    // Initialize I2C
    imu->wire->begin();
    
    // Setup interrupt pin
    pinMode(int_pin, INPUT);
    
    delay(10);  // Allow sensor to power up
    
    // Read chip ID
    uint16_t chip_id = readRegister16(imu, CHIP_ID_REG);
    if ((chip_id & 0x00) != BMI323_CHIP_ID) {
        Serial.printf("Invalid chip ID: 0x%02X (expected 0x%02X)\n", (chip_id & 0xFF), BMI323_CHIP_ID);
        return false;
    }
    
    // Soft reset
    writeRegister16(imu, CMD_REG, SOFT_RESET_CMD);
    delay(50);  // Wait for reset to complete
    
    // Configure accelerometer: 100Hz, normal mode
    writeRegister16(imu, ACC_CONF_REG, ACC_CONF_NORMAL_100HZ_8G);
    delay(10);
    
    // Configure gyroscope: 100Hz, ±2000 deg/s
    writeRegister16(imu, ACC_CONF_REG + 1, GYR_CONF_NORMAL_100HZ_2000DPS);
    delay(10);
    
    // Initialize feature engine
    if (!initializeFeatureEngine(imu)) {
        Serial.println("Feature engine initialization failed");
    }
    
    delay(50);  // Allow configuration to settle
    
    imu->initialized = true;
    Serial.println("BMI323 initialized successfully");
    return true;
}

bool imu_read_accel(imu_t* imu, float* x, float* y, float* z) {
    if (!imu || !imu->initialized) {
        return false;
    }
    
    uint16_t raw_x = readRegister16(imu, ACC_DATA_X_REG);
    uint16_t raw_y = readRegister16(imu, ACC_DATA_Y_REG);
    uint16_t raw_z = readRegister16(imu, ACC_DATA_Z_REG);
    
    if (x) *x = convertAccelData(raw_x);
    if (y) *y = convertAccelData(raw_y);
    if (z) *z = convertAccelData(raw_z);
    
    return true;
}

bool imu_read_gyro(imu_t* imu, float* x, float* y, float* z) {
    if (!imu || !imu->initialized) {
        return false;
    }
    
    uint16_t raw_x = readRegister16(imu, GYR_DATA_X_REG);
    uint16_t raw_y = readRegister16(imu, GYR_DATA_Y_REG);
    uint16_t raw_z = readRegister16(imu, GYR_DATA_Z_REG);
    
    if (x) *x = convertGyroData(raw_x);
    if (y) *y = convertGyroData(raw_y);
    if (z) *z = convertGyroData(raw_z);
    
    return true;
}

bool imu_read_temp(imu_t* imu, float* temp) {
    if (!imu || !imu->initialized || !temp) {
        return false;
    }
    
    uint16_t raw_temp = readRegister16(imu, TEMP_DATA_REG);
    *temp = convertTempData(raw_temp);
    
    return true;
}

bool imu_read(imu_t* imu, imu_data_t* data) {
    if (!imu || !imu->initialized || !data) {
        return false;
    }
    
    // Read sensor data
    uint16_t accX = readRegister16(imu, ACC_DATA_X_REG);
    uint16_t accY = readRegister16(imu, ACC_DATA_Y_REG);
    uint16_t accZ = readRegister16(imu, ACC_DATA_Z_REG);
    uint16_t gyrX = readRegister16(imu, GYR_DATA_X_REG);
    uint16_t gyrY = readRegister16(imu, GYR_DATA_Y_REG);
    uint16_t gyrZ = readRegister16(imu, GYR_DATA_Z_REG);
    uint16_t tempRaw = readRegister16(imu, TEMP_DATA_REG);
    
    // Convert to physical units
    data->accel_x = convertAccelData(accX);
    data->accel_y = convertAccelData(accY);
    data->accel_z = convertAccelData(accZ);
    data->gyro_x = convertGyroData(gyrX);
    data->gyro_y = convertGyroData(gyrY);
    data->gyro_z = convertGyroData(gyrZ);
    data->temp = convertTempData(tempRaw);
    
    return true;
}

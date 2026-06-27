#ifndef SHT40_H
#define SHT40_H

#include <stdint.h>
#include <stdbool.h>

// Error codes for professional status reporting 
typedef enum {
    SHT40_OK = 0,
    SHT40_ERR_NULL_PTR,
    SHT40_ERR_I2C_WRITE,
    SHT40_ERR_I2C_READ,
    SHT40_ERR_CRC_TEMP,
    SHT40_ERR_CRC_RH
} sht40_error_t;


// Measurement precision commands from Datasheet Page 11 (Section 4.5 Command Overview )
typedef enum {
    SHT40_PREC_HIGH   = 0xFD,
    SHT40_PREC_MEDIUM = 0xF6,
    SHT40_PREC_LOW    = 0xE0
} sht40_precision_t;

// Hardware Abstraction Layer (HAL): Function pointers for I2C and Delay.
// This allows testing on PC and running on any microcontroller later.
typedef bool (*sht40_i2c_write_fn)(uint8_t dev_addr, const uint8_t *data, uint16_t len);
typedef bool (*sht40_i2c_read_fn)(uint8_t dev_addr, uint8_t *data, uint16_t len);
typedef void (*sht40_delay_ms_fn)(uint32_t ms);

// Main sensor structure holding configuration and latest values
typedef struct {
    uint8_t i2c_address;           // SHT40 I2C address (0x44 , see Datasheet Page 1, Device Overview)
    sht40_i2c_write_fn i2c_write;  // Pointer to the I2C write function
    sht40_i2c_read_fn i2c_read;    // Pointer to the I2C read function
    sht40_delay_ms_fn delay_ms;    // Pointer to the delay function
    float temperature;             // Temperature in Celsius
    float humidity;                // Relative humidity in %
} sht40_t;

// Initialize the driver struct with the I2C functions
sht40_error_t sht40_init(sht40_t *dev, uint8_t i2c_addr, sht40_i2c_write_fn write, sht40_i2c_read_fn read, sht40_delay_ms_fn delay);

// Trigger a measurement with selected precision, verify checksums, and update values
sht40_error_t sht40_read_measurement(sht40_t *dev, sht40_precision_t precision);

#endif // SHT40_H

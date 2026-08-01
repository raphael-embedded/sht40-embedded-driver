#include "sht40.h"
#include <stddef.h>

// Datasheet Link: https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf

// CRC-8 specifications from Datasheet Page 11 (Section 4.4 Checksum Calculation):
#define SHT40_CRC8_POLYNOMIAL 0x31
#define SHT40_CRC8_INIT       0xFF

// Internal helper function to calculate Sensirion CRC8
static uint8_t calculate_crc8(const uint8_t *data, uint16_t len) {
    uint8_t crc = SHT40_CRC8_INIT;
    
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ SHT40_CRC8_POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

sht40_error_t sht40_init(sht40_t *dev, uint8_t i2c_addr, sht40_i2c_write_fn write, sht40_i2c_read_fn read, sht40_delay_ms_fn delay) {
    // Safety check: Make sure no NULL pointers were passed
    if (dev == NULL || write == NULL || read == NULL || delay == NULL) {
        return SHT40_ERR_NULL_PTR;
    }
    dev->i2c_address = i2c_addr;
    dev->i2c_write = write;
    dev->i2c_read = read;
    dev->delay_ms = delay;
    dev->temperature = 0.0f;
    dev->humidity = 0.0f;
    return SHT40_OK;
}

sht40_error_t sht40_read_measurement(sht40_t *dev, sht40_precision_t precision) {
    if (dev == NULL || dev->i2c_write == NULL || dev->i2c_read == NULL || dev->delay_ms == NULL) {
        return SHT40_ERR_NULL_PTR;
    }

    uint8_t cmd = (uint8_t)precision;
    
    // Send measurement command to sensor
    if (!dev->i2c_write(dev->i2c_address, &cmd, 1)) {
        return SHT40_ERR_I2C_WRITE;
    }

    // Wait for measurement to finish.
    // See Datasheet Page 10, Table 5 (Section 3.1 Timings): 
    // High-precision takes max 8.3ms (10ms safe). Med: max 4.5ms (6ms safe). Low: max 1.6ms (3ms safe).
    uint32_t delay_time = 10;
    if (precision == SHT40_PREC_MEDIUM) {
        delay_time = 6;
    } else if (precision == SHT40_PREC_LOW) {
        delay_time = 3;
    }
    dev->delay_ms(delay_time);

    // Read 6 bytes from the sensor.
    // See Datasheet Page 11 (Section 4.3 Data type & length): 
    // rx_buf[0] & rx_buf[1] = Temperature Ticks
    // rx_buf[2]             = Temperature CRC
    // rx_buf[3] & rx_buf[4] = Humidity Ticks
    // rx_buf[5]             = Humidity CRC
    uint8_t rx_buf[6] = {0};
    if (!dev->i2c_read(dev->i2c_address, rx_buf, 6)) {
        return SHT40_ERR_I2C_READ;
    }

    // Verify CRC checksums for data integrity
    if (calculate_crc8(&rx_buf[0], 2) != rx_buf[2]) {
        return SHT40_ERR_CRC_TEMP;
    }
    if (calculate_crc8(&rx_buf[3], 2) != rx_buf[5]) {
        return SHT40_ERR_CRC_RH;
    }

    // Combine bytes into 16-bit raw values
    uint16_t t_ticks = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    uint16_t rh_ticks = ((uint16_t)rx_buf[3] << 8) | rx_buf[4];

    // See Datasheet Page 12 (Section 4.6 Conversion of Signal Output): 
    // Formula T:  -45 + 175 * (S_T / (2^16 - 1))
    // Formula RH: -6 + 125 * (S_RH / (2^16 - 1))
    dev->temperature = -45.0f + 175.0f * ((float)t_ticks / 65535.0f);
    dev->humidity = -6.0f + 125.0f * ((float)rh_ticks / 65535.0f);

    // Keep humidity strictly between 0% and 100%
    if (dev->humidity > 100.0f) {
        dev->humidity = 100.0f;
    } 
    if (dev->humidity < 0.0f) {
        dev->humidity = 0.0f;
    }

    return SHT40_OK;
}

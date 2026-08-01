#include <stdio.h>
#include "../src/sht40.h"

// Global control variable to simulate different hardware behaviors in the mock
typedef enum {
    MOCK_MODE_SUCCESS,
    MOCK_MODE_I2C_FAIL,
    MOCK_MODE_BAD_CRC
} mock_behavior_t;

static mock_behavior_t current_mock_mode = MOCK_MODE_SUCCESS;

// Local helper to generate valid CRC8 bytes for the software mock environment
static uint8_t mock_calc_crc8(const uint8_t *data, uint16_t len) {
    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc = (crc << 1);
        }
    }
    return crc;
}

// --- SOFTWARE TESTS (MOCKS) ---

// Simulates sending I2C command
bool mock_i2c_write(uint8_t dev_addr, const uint8_t *data, uint16_t len) {
    if (current_mock_mode == MOCK_MODE_I2C_FAIL) {
        printf("[MOCK I2C] Write failed (Simulated Bus Error)\n");
        return false;
    }
    if (len > 0) {
        printf("[MOCK I2C] Sent command 0x%02X to address 0x%02X\n", data[0], dev_addr);
    }
    return true;
}

// Simulates reading data. We send fixed values back to test the formulas.
// For 25 degC -> T_ticks should be 26214 (0x6666)
// For 50 % RH  -> RH_ticks should be 29360 (0x72B0)
bool mock_i2c_read(uint8_t dev_addr, uint8_t *data, uint16_t len) {
    if (current_mock_mode == MOCK_MODE_I2C_FAIL) {
        printf("[MOCK I2C] Read failed (Simulated Bus Error)\n");
        return false;
    }

    if (len >= 6) {
        data[0] = 0x66; // Temp MSB
        data[1] = 0x66; // Temp LSB
        
        data[3] = 0x72; // Hum MSB
        data[4] = 0xB0; // Hum LSB

        if (current_mock_mode == MOCK_MODE_BAD_CRC) {
            data[2] = 0x00; // Temp CRC (Corrupted)
            data[5] = 0x00; // Hum CRC (Corrupted)
            printf("[MOCK I2C] Loaded simulated bytes with CORRUPTED CRC from address 0x%02X\n", dev_addr);
        } else {
            data[2] = mock_calc_crc8(&data[0], 2); // Valid Temp CRC
            data[5] = mock_calc_crc8(&data[3], 2); // Valid Hum CRC
            printf("[MOCK I2C] Loaded simulated bytes with VALID CRC from address 0x%02X\n", dev_addr);
        }
        return true;
    }
    return false;
}

// Simulates delay without actual long waiting time
void mock_delay_ms(uint32_t ms) {
    printf("[MOCK DELAY] Waiting for %u ms\n", ms);
}

// --- MAIN PROGRAM ---
int main(void) {
    sht40_t sensor;
    sht40_error_t err;
    
    printf("=== RUNNING SHT40 SOFTWARE TEST ===\n\n");

    // Initialize driver with our test functions
    err = sht40_init(&sensor, 0x44, mock_i2c_write, mock_i2c_read, mock_delay_ms);
    if (err != SHT40_OK) {
        printf("Initialization failed! Error code: %d\n", err);
        return 1;
    }

    // --- TEST 1: SUCCESSFUL RUN ---
    printf("--- Running Test 1: Successful Measurement ---\n");
    current_mock_mode = MOCK_MODE_SUCCESS;
    err = sht40_read_measurement(&sensor, SHT40_PREC_HIGH);
    if (err == SHT40_OK) {
        printf("\n[RESULT] TEST SUCCESSFUL!\n");
        printf("Temperature: %.2f degC (Expected: ~25.00)\n", sensor.temperature);
        printf("Humidity:    %.2f percentRH (Expected: ~50.00)\n", sensor.humidity);
    } else {
        printf("\n[RESULT] TEST FAILED! Error code: %d\n", err);
    }
    printf("\n\n");

    // --- TEST 2: I2C BUS FAILURE ---
    printf("--- Running Test 2: Simulated I2C Bus Failure ---\n");
    current_mock_mode = MOCK_MODE_I2C_FAIL;
    err = sht40_read_measurement(&sensor, SHT40_PREC_HIGH);
    if (err == SHT40_ERR_I2C_WRITE || err == SHT40_ERR_I2C_READ) {
        printf("\n[RESULT] TEST SUCCESSFUL! Driver correctly detected I2C failure.\n");
    } else {
        printf("\n[RESULT] TEST FAILED! Driver did not catch I2C failure. Code: %d\n", err);
    }
    printf("\n\n");

    // --- TEST 3: CORRUPTED CRC ---
    printf("--- Running Test 3: Simulated Corrupted CRC ---\n");
    current_mock_mode = MOCK_MODE_BAD_CRC;
    err = sht40_read_measurement(&sensor, SHT40_PREC_HIGH);
    if (err == SHT40_ERR_CRC_TEMP || err == SHT40_ERR_CRC_RH) {
        printf("\n[RESULT] TEST SUCCESSFUL! Driver correctly detected corrupted checksum.\n");
    } else {
        printf("\n[RESULT] TEST FAILED! Driver accepted bad data. Code: %d\n", err);
    }
    printf("\n\n");

    return 0;
}

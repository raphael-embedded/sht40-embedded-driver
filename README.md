# 🌡️ sht40-embedded-driver

A portable C driver for the Sensirion SHT40 relative humidity and temperature sensor.

## 🛠️ Project Status: Feature Complete & Tested

The core driver implementation and the software verification environment are complete. The driver has been successfully validated against communication errors and checksum corruption using a local hardware mock interface.

## ✨ Features

- 🔌 Hardware-independent architecture using function pointers (HAL approach)
- 🎯 Support for SHT40 measurement precision modes
- ✅ CRC-8 checksum verification according to Sensirion datasheet
- ⚠️ Robust I2C communication error handling
- 🌡️ Precise temperature and relative humidity conversion
- 🧪 Automated software test suite (Mock environment) for PC-based validation

## 📁 Project Structure

sht40-embedded-driver
├── src
│   ├── sht40.h       # Driver interface (HAL & API definitions)
│   └── sht40.c       # Driver implementation (CRC & math formulas)
├── tests
│   └── main.c        # Software mock test suite
└── README.md

## 🧩 Hardware Abstraction Layer (HAL)

The driver does not depend on a specific microcontroller architecture. Hardware-specific functions are injected dynamically via function pointers:

- sht40_i2c_write_fn: Custom I2C transmission
- sht40_i2c_read_fn: Custom I2C reception
- sht40_delay_ms_fn: Platform-specific millisecond timer

## 📊 Software Architecture

```mermaid
graph TD

%% Styling
classDef test fill:#e1f5fe,stroke:#0288d1,stroke-width:2px,color:#000;
classDef core fill:#efebe9,stroke:#5d4037,stroke-width:2px,color:#000;
classDef struct fill:#fff9c4,stroke:#fbc02d,stroke-width:2px,color:#000;


%% Test Environment
subgraph Test_Environment ["tests/main.c - PC Software Mock Environment"]

    M_Main["main.c Test Runner"]

    M_Mock["mock_i2c_write<br/>mock_i2c_read<br/>mock_delay_ms"]

    M_Main -->|Injects HAL callbacks| M_Init["sht40_init"]
    M_Main -->|Controls test scenarios| M_Mock

end


%% Driver Core
subgraph Driver_Core ["src/sht40.c & src/sht40.h - Hardware Independent Driver"]

    M_Init -->|Stores configuration and function pointers| D_Struct["sht40_t Struct"]

    D_Read["sht40_read_measurement"]

    D_Struct -->|Provides HAL interface| D_Read

    D_Read -->|Checks data integrity| D_CRC["calculate_crc8"]

    D_Read -->|Applies Sensirion conversion formulas| D_Convert["Temperature and Humidity Conversion"]

end


%% HAL connection
M_Mock -.->|Called through stored function pointers| D_Struct


%% Classes
class M_Main,M_Mock test;
class M_Init,D_Read,D_CRC,D_Convert core;
class D_Struct struct;
```

## 🧪 How to Run Software Tests

The project includes a standalone test suite in tests/main.c that simulates a virtual SHT40 sensor. It validates the mathematical conversion formulas and injects simulated bus errors and corrupted CRC checksums to verify driver stability.

To compile and run the tests on your local PC using any standard C compiler (e.g., GCC):

gcc src/sht40.c tests/main.c -o sht40_test
./sht40_test

### 📊 Expected Test Output

The test execution will run through three distinct test modes:
1. Successful Measurement: Validates 25.00 degC and 50.00 percentRH outputs using predefined data-register ticks.
2. Simulated I2C Bus Failure: Confirms that the driver gracefully handles communication aborts.
3. Simulated Corrupted CRC: Confirms that the driver detects and drops manipulated hardware bytes.

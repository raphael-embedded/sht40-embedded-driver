# sht40-embedded-driver

A portable C driver for the Sensirion SHT40 relative humidity and temperature sensor.


## 🛠️ Project Status: Work in Progress

The basic driver implementation is complete.

The driver is designed to be hardware-independent and can later be tested on a PC using a software mock environment before being deployed on a microcontroller.


## ✨ Features

- 🔌 Hardware-independent architecture using function pointers (HAL approach)
- 🎯 Support for SHT40 measurement precision modes
- ✅ CRC-8 checksum verification according to Sensirion datasheet
- ⚠️ I2C communication error handling
- 🌡️ Temperature and relative humidity conversion from raw sensor values
- 💻 Portable C implementation for embedded systems


## 🧩 Hardware Abstraction Layer (HAL)

The driver does not depend on a specific microcontroller.

Hardware-specific functions are provided through function pointers:

- 🔄 I2C write function
- 📥 I2C read function
- ⏱️ Millisecond delay function

This allows the same driver code to be adapted to different microcontroller platforms.


## 📁 Project Structure

```
sht40-embedded-driver
│
├── src
│   ├── sht40.h       # Driver interface
│   └── sht40.c       # Driver implementation
│
└── README.md
```


## 🚀 Next Step

- 🧪 Add software mock tests without requiring physical hardware

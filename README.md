# ESP32-S3 Zephyr Wi-Fi Socket
> **WiFi UDP/TCP Socket Communication using ESP32S3 and Zephyr RTOS 4.0.0**

![Zephyr RTOS](https://img.shields.io/badge/Zephyr_RTOS-4.0.0-blue?logo=zephyrproject&style=for-the-badge)
![Hardware](https://img.shields.io/badge/Hardware-ESP32_S3-red?logo=espressif&style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)

## 📖 Overview
This repository implements a Wi-Fi socket driver for the ESP32-S3 Development Kit C. It uses Zephyr RTOS v4.0.0 to handle UDP/TCP data transmission.

**Key Features:**
*  **Dual-Mode Networking:** Support for both TCP Server and UDP Client/Server modes.
*  **Python Testing Suite:** Includes `script_tcp_sender.py` and `script_udp_sender.py` for immediate loopback testing.

## 📂 Project Structure
The project follows a modular Zephyr workspace layout:

```text
west-workspace/
├── application/                # Main Application Logic
│   ├── app/
│   │   ├── src/                # C++ Source Code (main.cpp)
│   │   ├── CMakeLists.txt      # Build Configuration
│   │   └── prj.conf            # Kconfig Defaults
│   ├── zephyr/                 # Module Definitions
│   ├── scripts/                # Python Test Tools
│   │   ├── script_tcp_sender.py
│   │   └── script_udp_sender.py
│   └── west.yml                # Main Manifest
│
└── modules/                    # Zephyr Modules (HALs, SDKs)
```

## 🛠️ Prerequisites
* **Hardware:** ESP32-S3 DevKitC
* **OS:** Linux (Ubuntu 22.04)
* **Toolchain:** Zephyr SDK & West

## 🚀 Getting Started

### 1. Workspace Setup
Navigate to your workspace and configure the manifest path.

```bash
cd ~/zephyr-workspace/.west

# Edit the 'config' file to point to the application manifest
# Add/Modify the following lines:
[manifest]
path = application
file = west.yml
```

### 2. Install Dependencies
Pull the required Zephyr modules and Espressif HAL blobs.

```bash
cd ~/zephyr-workspace
west update -n
west blobs fetch hal_espressif
```
### 3. Configuration
Modify application/app/prj.conf to toggle between UDP and TCP modes before building.

```Properties
# Example Configuration
CONFIG_NET_TCP=y  
CONFIG_NET_UDP=y  
CONFIG_WIFI_SSID="Your_SSID"  
CONFIG_WIFI_PASSWORD="Your_Password"  
```

### 4. Build & Flash
Compile the firmware for the ESP32-S3 procpu core.

```bash
# Build the application
west build -p -b esp32s3_devkitc/esp32s3/procpu application/app

# Flash to the board
west flash

# Open Serial Monitor
west espressif monitor
```

## 🧪 Testing & Validation
Once the board is running and connected to Wi-Fi (check the Serial Monitor for the IP address), use the included Python scripts to verify data transmission.

```bash
# For TCP Testing:
python3 application/scripts/script_tcp_sender.py
#  For UDP Testing:
python3 application/scripts/script_udp_sender.py
```

---
**Maintained by D93 AIoT Solutions**
*Delivering End-to-End Solutions in Embedded Systems, AI, Robotics & Full-Stack Development.*

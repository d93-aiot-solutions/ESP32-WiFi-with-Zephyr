# WIFI_Socket_With_ESP32S3_Zephyr
This repository provides a simple demonstration of using Zephyr to control the Wi-Fi module of the ESP32S3 module for UDP/TCP socket data transmission. 

The Zephyr version used is 4.0.0. 

The ESP32S3 module used in this project is ESP32S3 Development Kit C.

**Project Structure**
###
        west-workspace/
        │
        ├── application/
        │   ├── app/              
        │   │   └── boards
        │   │   └── lib
        │   │   └── src/
        │   |       └── main.cpp
        │   │   └── CMakeLists.txt
        │   │   └── prj.conf
        │   ├── zephyr/                  
        │   │   └── module.yml          # The "west build" system will go to this "module.yml" and learn how to build the app
        │   ├── CMakeLists.txt 
        │   ├── Kconfig
        │   ├── README.md
        │   ├── script_tcp_sender.py
        │   ├── script_udp_sender.py
        │   ├── TODO.md
        │   ├── west.yml                 # Main manifest with optional import(s) and override(s)
        |                                                
        ├── modules                     
        │ 
        └── zephyr                       # The main Zephyr RTOS repository

**Prepare Environment**
- Go to _"zephyr-workspace/.west"_

- Add the following lines in to the file _"config"_: 
###
        [manifest]
        path = application
        file = west.yml

- This step will allow the command _"west update -n"_ to go to the _"west.yml"_ file in the folder _"application"_ to download necessary dependencies.

- Download necessary dependencies
###
        cd ~/zephyr-workspace
        west update -n
        west blobs fetch hal_espressif

**How To Use It**
- Modify the prj.conf to use either UDP or TCP (at server mode)

- Build the project and flash it to the chip
###
        west build -p -b esp32s3_devkitc/esp32s3/procpu application/app

        west flash

- Monitor the project to get its IP
###
        west espressif monitor

- Use the corresponding python script to communicate with ESP32S3 module via UDP or TCP socket with the corresponding IP
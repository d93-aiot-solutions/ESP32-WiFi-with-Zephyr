# WIFI_Socket_With_ESP32S3_Zephyr
This repository provides a simple demonstration of using Zephyr to control the Wi-Fi module of the ESP32S3 module for UDP/TCP socket data transmission. 

### Tasks
- [ ] Enable IPv6 Address

- [ ] How to use: #define NET_EVENT_WIFI_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED) and other events, e.g., IPV4_ADR_ADD. Possibly, it can't be done right now due to version compatibilities

- [ ] Delete TCP object when the port on the computer site is still opened

- [ ] Tasks masked with [TODO] inside the code
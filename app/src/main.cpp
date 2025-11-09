/******************************************************************************
Module: MAIN.CPP

Description: This project attempts to control an ESP32S3 module via WiFi using Zephyr. 
             The ESP32S3 will connect to a known wifi network, and open TCP or UDP socket 
             to connect to it. Two python scripts can be used to transfer the data to the 
             ESP32S3 devices using these two protocol
******************************************************************************/
/******************************************************************************
  INCLUDE
 *****************************************************************************/
// Zephyr RTOS
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/devicetree.h>

// Standard Library
#include <cstring>
#include <memory>

// Customized Library
#include "led.h"
#include "wifi.h"
#include "udp.h"
#include "tcp.h"



/******************************************************************************
  LOGGING SETUP
 *****************************************************************************/
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);



/******************************************************************************
  RGB LED - WS2812
 *****************************************************************************/
// NOTE: Although the PCB has only one LED, the built-in LED strip driver of 
//       Zephyr is used. Thus, existing "strip-related" variable like "RGB_LED_NUM_PIXELS"

#define RGB_LED_NODE	DT_ALIAS(rbg_led)

#if DT_NODE_HAS_PROP(DT_ALIAS(rbg_led), chain_length)
#define RGB_LED_NUM_PIXELS	DT_PROP(DT_ALIAS(rbg_led), chain_length)
#else
#error Unable to determine length of LED strip
#endif

static struct led_rgb pixels[RGB_LED_NUM_PIXELS];

static const struct device *const rgb_led = DEVICE_DT_GET(RGB_LED_NODE);

std::unique_ptr<SINGLE_RGB_LED_WS2812> rgb_led_ptr; // Object for the RGB LED



/******************************************************************************
  WIFI
 *****************************************************************************/
// [TODO]: Input the WIFI ID and password of your network
#define WIFI_SSID "wifi_ssid"     
#define WIFI_PSK  "wifi_password"



/******************************************************************************
  UDP
 *****************************************************************************/
// [TODO]: Modify the port for your application here and the python script
#define UDP_SERVER_PORT 4321



/******************************************************************************
  TCP
 *****************************************************************************/
// [TODO]: Modify the port for your application here and the python script
#define TCP_SERVER_PORT 4321



/******************************************************************************
  MAIN
 *****************************************************************************/
int main(void)
{

#if defined(CONFIG_USING_UDP) && defined(CONFIG_USING_TCP)
  LOG_ERR("The current development doesn't allow UDP and TCP running at the same time");
  return 0;
#endif

  // ========================= RGB LED =============================== //

  // Display board information
  LOG_INF("The board that we are working with is: %s", CONFIG_BOARD);

  // Check availability of the RGB LED
  if (device_is_ready(rgb_led)) 
  {
		LOG_INF("Found LED strip device %s", rgb_led->name);

    // Create the unique pointer for 
    rgb_led_ptr = std::make_unique<SINGLE_RGB_LED_WS2812>(rgb_led, pixels);
	} 
  else 
  {
		LOG_ERR("LED strip device %s is not ready", rgb_led->name);
		return 0;
	}

  // Turn the LED to RED indicate WIFI connection status, which is "disconnected"
  rgb_led_ptr->set_color_for_rgb_led(color_for_led_rgb::RED);

  // ========================= WIFI =============================== //

  // This sleep is used in the WIFI example
  LOG_INF("Sleep for 5s to follow the WIFI example initialization procedure...");
  k_sleep(K_SECONDS(5));

  // Create the WIFI object
  WIFI_STA_NETWORK wifi_sta_net(WIFI_SSID, WIFI_PSK, rgb_led_ptr.get());

  // Initialize the WIFI object and register for callback event
  wifi_sta_net.initialize_network();

  // ========================= MAIN LOOP =============================== //
  while (1)
  {
    // This function will block main.cpp until an IPv4 address is given to the ESP32S3, i.e., the WIFI connection is done
     wifi_sta_net.wait_for_ip();

     {      
      // ========================= UDP =============================== //
#if defined(CONFIG_USING_UDP)
      // Create UDP object
      UDP_SERVER udp_server(UDP_SERVER_PORT, rgb_led_ptr.get());
        
      // Start the UDP server
      udp_server.start_udp_server();
#endif 

      // ========================= TCP =============================== //

#if defined(CONFIG_USING_TCP)
      // Create TCP object
      TCP_SERVER tcp_server(TCP_SERVER_PORT, rgb_led_ptr.get());
        
      // Start the TCP server
      tcp_server.start_tcp_server();
#endif 

      // The loop will wait here for disconnection semaphore 
      wifi_sta_net.wait_for_wifi_to_disconnect();
    }

  }

	return 0;
}



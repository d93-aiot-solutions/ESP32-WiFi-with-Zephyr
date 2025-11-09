#ifndef LIB_WIFI_H
#define LIB_WIFI_H
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/sys/util.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/conn_mgr_connectivity.h>

// Project specific headers
#include "led.h"



/******************************************************************************
WIFI CLASS
******************************************************************************/
class WIFI_STA_NETWORK
{
public:
    // Constructor
    WIFI_STA_NETWORK(const char* ssid, const char* psk, SINGLE_RGB_LED_WS2812* rgb_led);

    // Destructor
    ~WIFI_STA_NETWORK();

    // Functions to initialize the network
    void initialize_network(void);

    // Functions to connect to a WIFI
    int connect_to_wifi(void);

    // Functions to wait untile the WIFI connection is established
    void wait_for_ip(void);

    // A pending function that put on main.cpp to notify its about the WIFI disconnection
    void wait_for_wifi_to_disconnect(void);

    // Variable to indicate the connection status
    bool m_is_connected;

private:
    // Member variables to store the network credentials
    const char* m_ssid;
    const char* m_psk;

    // STA interface
    struct net_if *m_sta_iface;

    // STA configuration
    struct wifi_connect_req_params m_sta_config;

    // Callback for the network management event
    struct net_mgmt_event_callback m_cb;

    // LED indicator
    SINGLE_RGB_LED_WS2812* m_led_indicator;

    // Semaphore to indicate the wifi connection is established 
    struct k_sem m_ip_ready_sem;

    // Semaphore to indicate the wifi connection is ended
    struct k_sem m_wifi_disconnected_sem;

    // The static wrapper function that Zephyr's C API will call
    static void static_wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface);

    // The non-static (instance) handler where your actual logic goes
    void wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface);

    // Reconnect work item
    struct k_work_delayable m_reconnect_work;

    // Reconnect work handler
    static void static_reconnect_work_handler(struct k_work *work);

};

#endif // LIB_WIFI_H
/******************************************************************************
Module: WIFI.CPP

Description: This file contains functions that allows the esp32s3 to interact
             with WIFI network
******************************************************************************/
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

// Project specific headers
#include "wifi.h"
#include "led.h"

// Standard Library
#include <cstring>



/******************************************************************************
  DEFINE
 *****************************************************************************/
#define NET_EVENT_WIFI_MASK (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)


/******************************************************************************
  LOGGING SETUP
 *****************************************************************************/
LOG_MODULE_REGISTER(wifi, LOG_LEVEL_INF);



/******************************************************************************
FUNCTIONS DEFINITIONS
******************************************************************************/
/**
 * @brief Constructor for the WIFI class
 */
WIFI_STA_NETWORK::WIFI_STA_NETWORK(const char* ssid, const char* psk, SINGLE_RGB_LED_WS2812* rgb_led)
    : m_ssid(ssid), m_psk(psk), m_led_indicator(rgb_led)
{
    // Init the semaphore that indicate the readiness of the wifi connection
    k_sem_init(&m_ip_ready_sem, 0, 1);

    // Init the semaphore that indicate the disconnection
    k_sem_init(&m_wifi_disconnected_sem, 0, 1);

    // Initialize the connection status
    m_is_connected = false;

    // Initialize the reconnect work
    k_work_init_delayable(&m_reconnect_work, static_reconnect_work_handler);
}

/**
 * @brief Destructor
 */
WIFI_STA_NETWORK::~WIFI_STA_NETWORK()
{
    // Tell the kernel to remove our callback from its list.
    LOG_INF("WIFI object is deleted and unregistering WIFI event callback.");
    net_mgmt_del_event_callback(&m_cb);
}


/**
 * @brief Initialize the network and start the event call back
 */
void WIFI_STA_NETWORK::initialize_network(void)
{
    // Initialize the wifi-related callback events
	net_mgmt_init_event_callback(&m_cb, static_wifi_event_handler, NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&m_cb);

    // Get the default (and only) Wi-Fi interface, which is the STA interface
    m_sta_iface = net_if_get_default();

    // This wait here is a workaround solution to connect to the Wi-Fi network
    // The correct solution should be check the status of "m_sta_iface".
    // However, for some reason, the status of this interface is always "down" even net_if_up() is called
    while (connect_to_wifi() != 0)
    {
        k_sleep(K_MSEC(500));
    }

    // Sleep another time to ensure the connection is stable and ready
    // [TODO]: Find a better way to do this. Maybe newer version of Zephyr has fixed this problem
    // Without this sleep, the connection sometimes failed. Developer is encouraged to investigate further.
    k_sleep(K_SECONDS(5));
}

/**
 * @brief Connect to the WIFI with preset password and ID
 */
int WIFI_STA_NETWORK::connect_to_wifi(void)
{
	if (!m_sta_iface) 
    {
		LOG_INF("STA: interface no initialized");
		return -EIO;
	}

	m_sta_config.ssid        = (const uint8_t *)m_ssid;
	m_sta_config.ssid_length = strlen(m_ssid);
	m_sta_config.psk         = (const uint8_t *)m_psk;
	m_sta_config.psk_length  = strlen(m_psk);
	m_sta_config.security = WIFI_SECURITY_TYPE_PSK;
	m_sta_config.channel  = WIFI_CHANNEL_ANY;
	m_sta_config.band     = WIFI_FREQ_BAND_2_4_GHZ;

	LOG_INF("Connecting to SSID: %s...", m_sta_config.ssid);

	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, m_sta_iface, &m_sta_config,
			   sizeof(struct wifi_connect_req_params));

	return ret;
}

/**
 * @brief This is the static "trampoline" function to help us bridge the gap between static C API of net_mgmt_init_event_callback and the C++ object
 */
void WIFI_STA_NETWORK::static_wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
    // [TODO]: Try to use the C++ way to pass the 'this' pointer to this function, which will remove the warning
    // Get the parent WIFI_STA_NETWORK object instance. We are getting the pointer to the 'WIFI_STA_NETWORK' struct that contains the 'm_cb' member.
    WIFI_STA_NETWORK *self = CONTAINER_OF(cb, WIFI_STA_NETWORK, m_cb);

    // Now, call the non-static (instance) member function using the 'self' (this) pointer.
    if (self) 
    {
        self->wifi_event_handler(cb, mgmt_event, iface);
    }
}

/**
 * @brief Handle the event callback for the network management
 */
void WIFI_STA_NETWORK::wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) 
    {
        // Connection result
        case NET_EVENT_WIFI_CONNECT_RESULT: 
        {   
            m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::YELLOW);
            // Cancel any pending reconnect work
            k_work_cancel_delayable(&m_reconnect_work);

            LOG_INF("Taking IPv4 address....");

            // [TODO] - Attempt to use the event flag instead of this timer
            // Explain: If there is no delay here, the net_if_get_config() will return NULL pointer
            k_sleep(K_SECONDS(10));

            // Variable to take IPv4 address
            struct net_if_config *cfg;
            char buf[NET_IPV4_ADDR_LEN];

            // Take network config 
            cfg = net_if_get_config(iface);
            if (!cfg) 
            {
                LOG_INF("Failed to take network configuration");
                break;
            }

            // We are supposed to loop through NET_IF_MAX_IPV4_ADDR to get all IPv4 address
            // However, this project set the max no. IPv4 addresses as 1 in prj.conf
            LOG_INF("The IPv4 address: %s",
                net_addr_ntop(AF_INET,
                        &cfg->ip.ipv4->unicast[0].ipv4.address.in_addr,
                        buf, sizeof(buf)));
            
            // Connection success log
            LOG_INF("Connected to %s", m_ssid);

            // Switch the connection status
            m_is_connected = true;

            //  Give the semaphore to unblock main()
            k_sem_give(&m_ip_ready_sem);
            
            // [TODO] - Attempt to use the event flag instead of this timer
            // Explain: If there is no delay here, the while loop in main() behaves erratically
            // Developer is encouraged to investigate further.
            k_sleep(K_SECONDS(10));

            break;
        }

        // Disconnection result
        case NET_EVENT_WIFI_DISCONNECT_RESULT: 
        {
            LOG_INF("Disconnection event is triggered.");

            if (m_is_connected == true)
            {
                // Switch the connection status
                m_is_connected = false;

                //  Give the semaphore to notify main about the wifi disconnection
                k_sem_give(&m_wifi_disconnected_sem);
            }

            // Change LED to red to indicate disconnection. The LED is turned green after the UDP/TCP is ready, which happens after connection is established.
            m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::RED);

            k_work_schedule(&m_reconnect_work, K_SECONDS(5));

            break;
        }

        default:
            break;
	}
}

/**
 * @brief This function will waits until the wifi connection is established, i.e., an IP is ready
 */
void WIFI_STA_NETWORK::wait_for_ip(void)
{
    LOG_INF("Waiting for IPv4 address, i.e., WIFI connection completed...");
    k_sem_take(&m_ip_ready_sem, K_FOREVER);
    LOG_INF("WIFI connection is established and IPv4 address is received.");
}

/**
 * @brief This function will waits until the wifi connection is established, i.e., an IP is ready
 */
void WIFI_STA_NETWORK::wait_for_wifi_to_disconnect(void)
{
    LOG_INF("Pending here until WIFI disconnection is detected...");
    k_sem_take(&m_wifi_disconnected_sem, K_FOREVER);
    LOG_INF("WIFI connection is lost.");
}

/**
 * @brief This function is to handle the reconnect work, which will attempt to reconnect to the WIFI network
 */
void WIFI_STA_NETWORK::static_reconnect_work_handler(struct k_work *work)
{
    // Get the 'self' pointer. We must use CONTAINER_OF to find the parent class that this k_work struct lives inside.
    // [TODO]: Try to use the C++ way to pass the 'this' pointer to this function, which will remove the warning
    WIFI_STA_NETWORK *self = CONTAINER_OF(work, WIFI_STA_NETWORK, m_reconnect_work);
    
    // Call the connect function
    LOG_INF("Attempting to reconnect to the WIFI network %s...", self->m_ssid);
    while (self->connect_to_wifi() != 0) 
    {
        k_sleep(K_MSEC(500));
    }

    // Sleep another time to ensure the connection is stable and ready
    // [TODO]: Find a better way to do this. Maybe newer version of Zephyr has fixed this problem
    // Without this sleep, the connection sometimes failed. Developer is encouraged to investigate further.
    k_sleep(K_SECONDS(5));
}
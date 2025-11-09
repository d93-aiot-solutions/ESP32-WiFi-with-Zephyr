/******************************************************************************
Module: UDP.CPP

Description: This file contains functions that allows the ESP32S3 send and receive
             data from other devices within the network via UDP
******************************************************************************/
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

// Project specific headers
#include "udp.h"



/******************************************************************************
  THREAD
 *****************************************************************************/
// [TODO] - Is it necessary to do like this? It we just define the stack anyway, will it impact the RAM?
K_THREAD_STACK_DEFINE(m_udp_thread_stack, UDP_STACK_SIZE);



/******************************************************************************
  LOGGING SETUP
 *****************************************************************************/
LOG_MODULE_REGISTER(udp, LOG_LEVEL_INF);



/******************************************************************************
FUNCTIONS DEFINITIONS
******************************************************************************/
/**
 * @brief Constructor for the UDP class
 */
UDP_SERVER::UDP_SERVER(uint16_t port, SINGLE_RGB_LED_WS2812* rgb_led)
    : m_sock(-1), m_port(port), m_led_indicator(rgb_led)
{
    // Initialize socket as -1 to indicate that it has not been initialized yet
}

/**
 * @brief Destructor for the UDP class
 */
UDP_SERVER::~UDP_SERVER()
{
    // Close the socket if it is open correctly
    if (m_sock >= 0) 
    {
        close(m_sock);
    }

    int ret = k_thread_join(&m_thread_data, K_FOREVER);
    if (ret) 
    {
        LOG_WRN("Failed to join UDP thread: %d", ret);
    }

    LOG_INF("UDP object is deleted and socket is closed.");
}

/**
 * @brief This function create the UDP thread and start it
 */
void UDP_SERVER::start_udp_server()
{
    // Create the thread. We pass 'this' as the first argument (p1) so our static wrapper can find its object.
    k_tid_t udp_server_thread = k_thread_create(&m_thread_data, m_udp_thread_stack,
                                  K_THREAD_STACK_SIZEOF(m_udp_thread_stack),
                                  UDP_SERVER::static_run_udp_server,
                                  this, NULL, NULL,
                                  UDP_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(udp_server_thread, "udp_server");
}


/**
 * @brief This function is a static wrapper for the actual function to run the udp thread
 */
void UDP_SERVER::static_run_udp_server(void *p1, void *p2, void *p3)
{
    // p1 contains the 'this' pointer we passed in k_thread_create
    UDP_SERVER* self = static_cast<UDP_SERVER*>(p1);
    
    // Call the real, non-static run method
    self->run_udp_server();
}

/**
 * @brief This is the actual function that will run the UDP thread logic
 */
void UDP_SERVER::run_udp_server()
{
    // Necessary variables
    struct sockaddr_in bind_addr;
    char buffer[128];

    // Create the socket
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sock < 0) 
    {
        LOG_ERR("Failed to create socket: %d", errno);
        return;
    }

    // Bind the socket to our port
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(m_port);
    if (bind(m_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) 
    {
        LOG_ERR("Failed to bind socket: %d", errno);
        close(m_sock);
        return;
    }

    // Waiting for UDP data
    LOG_INF("Listening UDP data on the port %d", m_port);

    // Set LED as green to indicate UDP server is running
    m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::GREEN);

    while (1)
    {
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        // Blocking function that wait for the upcoming udp data
        int recv_len = recvfrom(m_sock, buffer, sizeof(buffer) - 1, 0, &client_addr, &client_addr_len);

        if (recv_len > 0) 
        {
            buffer[recv_len] = '\0';
            LOG_INF("Received data: %s", buffer);
        } 
        else 
        {
            LOG_WRN("recvfrom failed: %d", errno);

            // Set LED as flashing red to indicate UDP server error
            m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::RED);

            break;
        }
    }
}

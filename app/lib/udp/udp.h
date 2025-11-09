#ifndef LIB_UDP_H
#define LIB_UDP_H
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

// Project specific headers
#include "led.h"


/******************************************************************************
DEFINE
******************************************************************************/
#define UDP_STACK_SIZE       2048
#define UDP_THREAD_PRIORITY  8


/******************************************************************************
UDP SERVER CLASS
******************************************************************************/
class UDP_SERVER
{
public:
    // Constructor
    UDP_SERVER(uint16_t port, SINGLE_RGB_LED_WS2812* rgb_led);
    
    // Destructor
    ~UDP_SERVER();

    // Start the UDP server
    void start_udp_server();

private:

    // Socket file descriptor and port to listen on
    int m_sock;            
    uint16_t m_port;     
    
    // Thread
    struct k_thread m_thread_data;

    // LED indicator
    SINGLE_RGB_LED_WS2812* m_led_indicator;

    // Functions to start running the udp server
    void run_udp_server();

    // Static function for the thread entry, which in turns call the actual "run_udp_server"
    static void static_run_udp_server(void *p1, void *p2, void *p3);
};

#endif // LIB_UDP_H
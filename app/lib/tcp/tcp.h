#ifndef LIB_TCP_H
#define LIB_TCP_H
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
#define TCP_STACK_SIZE       2048
#define TCP_THREAD_PRIORITY  8



/******************************************************************************
TCP SERVER CLASS
******************************************************************************/
class TCP_SERVER
{
public:
    // Constructor
    TCP_SERVER(uint16_t port, SINGLE_RGB_LED_WS2812* rgb_led);
    
    // Destructor
    ~TCP_SERVER();

    // Start the TCP server
    void start_tcp_server();

private:

    // Socket file descriptor and port to listen on
    int m_sock;   
    int m_client_sock;         
    uint16_t m_port;     
    
    // Thread
    struct k_thread m_thread_data;

    // LED indicator
    SINGLE_RGB_LED_WS2812* m_led_indicator;

    // Functions to start running the tcp server
    void run_tcp_server();

    // Static function for the thread entry, which in turns call the actual "run_tcp_server"
    static void static_run_tcp_server(void *p1, void *p2, void *p3);
};

#endif // LIB_TCP_H
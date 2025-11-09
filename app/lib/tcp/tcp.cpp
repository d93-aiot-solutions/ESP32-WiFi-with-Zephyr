/******************************************************************************
Module: TCP.CPP

Description: This file contains functions that allows the ESP32S3 send and receive
             data from other devices within the network via TCP
******************************************************************************/
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

// Project specific headers
#include "tcp.h"



/******************************************************************************
  THREAD
 *****************************************************************************/
// [TODO]: Does this consume RAM or anything?
K_THREAD_STACK_DEFINE(m_tcp_thread_stack, TCP_STACK_SIZE);



/******************************************************************************
  LOGGING SETUP
 *****************************************************************************/
LOG_MODULE_REGISTER(tcp, LOG_LEVEL_INF);



/******************************************************************************
  DEFINE
 *****************************************************************************/
// Defines the maximum number of pending connections the kernel will queue. '1' means we'll handle one client at a time.
#define TCP_LISTEN_BACKLOG 1



/******************************************************************************
FUNCTIONS DEFINITIONS
******************************************************************************/
/**
 * @brief Constructor for the TCP class
 */
TCP_SERVER::TCP_SERVER(uint16_t port, SINGLE_RGB_LED_WS2812* rgb_led)
    : m_sock(-1), m_client_sock(-1), m_port(port), m_led_indicator(rgb_led)
{
    // Initialize socket as -1 to indicate that it has not been initialized yet
}

/**
 * @brief Destructor for the TCP class
 */
TCP_SERVER::~TCP_SERVER()
{
    // Close the active client socket
    if (m_client_sock >= 0) 
    {
        close(m_client_sock);
    }

    // Close the active socket
    if (m_sock >= 0) 
    {
        close(m_sock);
    }

    int ret = k_thread_join(&m_thread_data, K_FOREVER);
    if (ret) 
    {
        LOG_WRN("Failed to join TCP thread: %d", ret);
    }

    LOG_INF("TCP object is deleted and socket is closed.");
}

/**
 * @brief This function create the TCP thread and start it
 */
void TCP_SERVER::start_tcp_server()
{
    // Create the thread. We pass 'this' as the first argument (p1) so our static wrapper can find its object.
    k_tid_t tcp_server_thread = k_thread_create(&m_thread_data, m_tcp_thread_stack,
                                  K_THREAD_STACK_SIZEOF(m_tcp_thread_stack),
                                  TCP_SERVER::static_run_tcp_server,
                                  this, NULL, NULL,
                                  TCP_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(tcp_server_thread, "tcp_server");
}

/**
 * @brief This function is a static wrapper for the actual function to run the tcp thread
 */
void TCP_SERVER::static_run_tcp_server(void *p1, void *p2, void *p3)
{
    // p1 contains the 'this' pointer we passed in k_thread_create
    TCP_SERVER* self = static_cast<TCP_SERVER*>(p1);
    
    // Call the real, non-static run method
    self->run_tcp_server();
}

/**
 * @brief This is the actual function that will run the TCP thread logic
 */
void TCP_SERVER::run_tcp_server()
{
    // Necessary variables
    struct sockaddr_in bind_addr;
    char buffer[128];
    
    // Variables for the client connection
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a TCP stream socket
    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sock < 0) 
    {
        LOG_ERR("Failed to create TCP socket: %d", errno);
        return;
    }

    // Bind the socket to our port
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(m_port);
    if (bind(m_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) 
    {
        LOG_ERR("Failed to bind TCP socket: %d", errno);
        close(m_sock);
        return;
    }

    // Put the socket into listening mode 
    if (listen(m_sock, TCP_LISTEN_BACKLOG) < 0)
    {
        LOG_ERR("Failed to listen on TCP socket: %d", errno);
        close(m_sock);
        return;
    }

    // TCP uses a nested loop 
    LOG_INF("Listening for TCP connections on port %d", m_port);

    // Set LED as green to indicate TCP server is running
    m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::GREEN);
    
    // Outer loop: Waits for new clients to connect
    while (1)
    {
        // Block and wait for a client to connect
        m_client_sock = accept(m_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (m_client_sock < 0)
        {
            LOG_ERR("Failed to accept connection: %d", errno);
            // Set LED as red to indicate TCP server error
            m_led_indicator->set_color_for_rgb_led(color_for_led_rgb::RED);
            break;
        }

        LOG_INF("TCP client connected");

        // Inner loop: Handle data from this *one* client
        while (1)
        {
            // Use recv() on the *client* socket 
            int recv_len = recv(m_client_sock, buffer, sizeof(buffer) - 1, 0);

            if (recv_len > 0) 
            {
                buffer[recv_len] = '\0';
                LOG_INF("Received data: %s", buffer);
            } 
            else if (recv_len == 0)
            {
                // Client closed the connection gracefully ---
                LOG_INF("TCP client disconnected");
                break; // Break from the *inner* loop
            }
            else 
            {
                // An error occurred on this connection ---
                LOG_WRN("recv failed: %d", errno);
                break; // Break from the *inner* loop
            }
        }
        
        // Close the *client's* socket
        // The outer loop will then wait for a new client
        close(m_client_sock);
        m_client_sock = -1;
    }
}

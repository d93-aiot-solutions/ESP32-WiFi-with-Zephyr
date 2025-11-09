/******************************************************************************
Module: LED.CPP

Description: This file contains functions of the led class, which is in charge
             of controlling the onboard RGB LED WS2812 chip
******************************************************************************/
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

// Project specific headers
#include "led.h"

// Standard Library
#include <cstring>



/******************************************************************************
  LOGGING SETUP
 *****************************************************************************/
LOG_MODULE_REGISTER(led, LOG_LEVEL_INF);



/******************************************************************************
FUNCTIONS DEFINITIONS FOR NETWORK CLASS
******************************************************************************/
/**
 * @brief Constructor for the LED class
 * The private variables, m_strip, m_pixels, and m_num_pixels, will be initialized.
 */
SINGLE_RGB_LED_WS2812::SINGLE_RGB_LED_WS2812(const struct device *strip_dev, struct led_rgb *pixel_buffer)
    : m_strip(strip_dev), m_pixels(pixel_buffer)
{
    
}

/**
 * @brief Set color of the LED with the input is 3-components color code
 */
void SINGLE_RGB_LED_WS2812::set_color_for_rgb_led(uint8_t r, uint8_t g, uint8_t b)
{
    struct led_rgb color = RGB(r, g, b);
    set_color_for_rgb_led(color);
}


/**
 * @brief Set the color of the LED with the input is the led_rgb color code
 */
void SINGLE_RGB_LED_WS2812::set_color_for_rgb_led(const struct led_rgb &color)
{
    // Set the color 
    m_pixels[0] = color;

    // Update the color
    int result = led_strip_update_rgb(m_strip, m_pixels, 1);

    // Show the result
    if (result) 
    {
		LOG_ERR("Couldn't update strip: %d", result);
	}
}
#ifndef LIB_LED_H
#define LIB_LED_H
/******************************************************************************
INCLUDE
******************************************************************************/
// Zephyr RTOS
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>

// Standard Library
#include <cstring>



/******************************************************************************
DEFINE
******************************************************************************/
#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }



/******************************************************************************
GLOBAL VARIABLES
******************************************************************************/
// Color used for the LED
namespace color_for_led_rgb 
{
constexpr struct led_rgb RED    = RGB(0x0f, 0x00, 0x00);
constexpr struct led_rgb GREEN  = RGB(0x00, 0x0f, 0x00);
constexpr struct led_rgb BLUE   = RGB(0x00, 0x00, 0x0f);
constexpr struct led_rgb YELLOW = RGB(0x0f, 0x0f, 0x00);
constexpr struct led_rgb OFF    = RGB(0x00, 0x00, 0x00);

}  // namespace color_for_led_rgb



/******************************************************************************
LED CLASS
******************************************************************************/
class SINGLE_RGB_LED_WS2812
{
public:
    // Constructor
    SINGLE_RGB_LED_WS2812(const struct device *strip_dev, struct led_rgb *pixel_buffer);

    // Functions to set color for the rgb led
    void set_color_for_rgb_led(uint8_t r, uint8_t g, uint8_t b);
    void set_color_for_rgb_led(const struct led_rgb &color);

private:

    const struct device *m_strip;      // Pointer to the LED strip device
    struct led_rgb      *m_pixels;     // Pointer to the pixel array (framebuffer)
};

#endif // LIB_LED_H
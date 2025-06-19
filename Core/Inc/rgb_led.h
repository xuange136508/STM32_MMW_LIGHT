#ifndef __RGB_LED_H__
#define __RGB_LED_H__

#include "main.h"
#include "tim.h"

/* RGB LED mode enumeration */
typedef enum {
    RGB_MODE_OFF = 0,           // Off
    RGB_MODE_STATIC_RED,        // Static red
    RGB_MODE_STATIC_GREEN,      // Static green  
    RGB_MODE_STATIC_BLUE,       // Static blue
    RGB_MODE_STATIC_WHITE,      // Static white
    RGB_MODE_BLINK_RED,         // Red blinking
    RGB_MODE_BLINK_GREEN,       // Green blinking
    RGB_MODE_BLINK_BLUE,        // Blue blinking
    RGB_MODE_BREATHING,         // Breathing effect
    RGB_MODE_RAINBOW,           // Rainbow gradient
    RGB_MODE_ALTERNATE,         // Dual LED alternating
    RGB_MODE_MAX
} RGB_Mode_t;

/* RGB color structure */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB_Color_t;

/* RGB LED control structure */
typedef struct {
    RGB_Mode_t mode;            // Current mode
    RGB_Color_t color;          // Current color
    uint32_t counter;           // Counter
    uint32_t period;            // Period
    uint8_t brightness;         // Brightness (0-100)
    uint8_t state;              // Current state
    uint8_t led_index;          // Active LED index (0 or 1)
} RGB_LED_t;

/* Function declarations */
void RGB_LED_Init(void);
void RGB_LED_SetMode(RGB_Mode_t mode);
void RGB_LED_SetColor(uint8_t red, uint8_t green, uint8_t blue);
void RGB_LED_SetBrightness(uint8_t brightness);
void RGB_LED_Update(void);
void RGB_LED_Test(void);

#endif /* __RGB_LED_H__ */
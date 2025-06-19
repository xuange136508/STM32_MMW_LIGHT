#include "rgb_led.h"
#include <stdio.h>
#include <math.h>

/* Private variables */
static RGB_LED_t rgb_led;
static uint32_t pwm_counter = 0;        // PWM software counter
static const uint32_t PWM_PERIOD = 100; // PWM period (100 brightness levels)

/* Private function declarations */
static void RGB_LED_SetOutput(uint8_t state);
static void RGB_LED_PWM_Update(void);
static void RGB_LED_Effect_Update(void);

/**
 * @brief RGB LED initialization
 */
void RGB_LED_Init(void)
{
    // Initialize RGB LED control structure
    rgb_led.mode = RGB_MODE_OFF;
    rgb_led.color.red = 0;
    rgb_led.color.green = 0;
    rgb_led.color.blue = 0;
    rgb_led.counter = 0;
    rgb_led.period = 100;        // Default period
    rgb_led.brightness = 50;     // Default brightness 50%
    rgb_led.state = 0;
    rgb_led.led_index = 0;
    
    // Turn off LED
    RGB_LED_SetOutput(0);
    
    // Start timer interrupt
    HAL_TIM_Base_Start_IT(&htim3);
    
    printf("RGB LED initialization completed\r\n");
}

/**
 * @brief Set RGB LED mode
 * @param mode: RGB LED mode
 */
void RGB_LED_SetMode(RGB_Mode_t mode)
{
    if (mode < RGB_MODE_MAX) {
        rgb_led.mode = mode;
        rgb_led.counter = 0;  // Reset counter
        printf("RGB LED mode set to: %d\r\n", mode);
    }
}

/**
 * @brief Set RGB color
 * @param red: Red component (0-255)
 * @param green: Green component (0-255) 
 * @param blue: Blue component (0-255)
 */
void RGB_LED_SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    rgb_led.color.red = red;
    rgb_led.color.green = green;
    rgb_led.color.blue = blue;
    printf("RGB color set to: R=%d, G=%d, B=%d\r\n", red, green, blue);
}

/**
 * @brief Set brightness
 * @param brightness: Brightness value (0-100)
 */
void RGB_LED_SetBrightness(uint8_t brightness)
{
    if (brightness <= 100) {
        rgb_led.brightness = brightness;
        printf("RGB brightness set to: %d%%\r\n", brightness);
    }
}

/**
 * @brief Set LED output state
 * @param state: Output state (0=off, 1=on)
 */
static void RGB_LED_SetOutput(uint8_t state)
{
    if (state) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

/**
 * @brief PWM software update
 */
static void RGB_LED_PWM_Update(void)
{
    pwm_counter++;
    if (pwm_counter >= PWM_PERIOD) {
        pwm_counter = 0;
    }
    
    // Calculate output based on current mode and brightness
    uint8_t output = 0;
    
    switch (rgb_led.mode) {
        case RGB_MODE_OFF:
            output = 0;
            break;
            
        case RGB_MODE_STATIC_RED:
        case RGB_MODE_STATIC_GREEN:
        case RGB_MODE_STATIC_BLUE:
        case RGB_MODE_STATIC_WHITE:
            output = (pwm_counter < rgb_led.brightness) ? 1 : 0;
            break;
            
        default:
            // Other modes handled by effect update function
            output = (pwm_counter < rgb_led.brightness && rgb_led.state) ? 1 : 0;
            break;
    }
    
    RGB_LED_SetOutput(output);
}

/**
 * @brief Light effect update
 */
static void RGB_LED_Effect_Update(void)
{
    rgb_led.counter++;
    
    switch (rgb_led.mode) {
        case RGB_MODE_OFF:
            rgb_led.state = 0;
            break;
            
        case RGB_MODE_STATIC_RED:
        case RGB_MODE_STATIC_GREEN:
        case RGB_MODE_STATIC_BLUE:
        case RGB_MODE_STATIC_WHITE:
            rgb_led.state = 1;
            break;
            
        case RGB_MODE_BLINK_RED:
        case RGB_MODE_BLINK_GREEN:
        case RGB_MODE_BLINK_BLUE:
            // Blinking effect: toggle state every 50 interrupts (~0.2s)
            if (rgb_led.counter >= 50) {
                rgb_led.state = !rgb_led.state;
                rgb_led.counter = 0;
            }
            break;
            
        case RGB_MODE_BREATHING:
            // Breathing effect: sine wave brightness control
            {
                float phase = (float)(rgb_led.counter % 200) / 200.0f * 2.0f * 3.14159f;
                rgb_led.brightness = (uint8_t)(50 + 49 * sin(phase));
                rgb_led.state = 1;
            }
            break;
            
        case RGB_MODE_RAINBOW:
            // Rainbow effect: cycle through colors
            {
                uint32_t color_phase = rgb_led.counter % 600;  // Rainbow period
                if (color_phase < 200) {
                    // Red->Green
                    rgb_led.brightness = 70;
                } else if (color_phase < 400) {
                    // Green->Blue  
                    rgb_led.brightness = 50;
                } else {
                    // Blue->Red
                    rgb_led.brightness = 90;
                }
                rgb_led.state = 1;
            }
            break;
            
        case RGB_MODE_ALTERNATE:
            // Dual LED alternating: switch LED every 30 interrupts
            if (rgb_led.counter >= 30) {
                rgb_led.led_index = !rgb_led.led_index;
                rgb_led.state = 1;
                rgb_led.counter = 0;
                printf("Switch to LED %d\r\n", rgb_led.led_index);
            }
            break;
            
        default:
            rgb_led.state = 0;
            break;
    }
}

/**
 * @brief RGB LED update function (called in timer interrupt)
 */
void RGB_LED_Update(void)
{
    // Update PWM
    RGB_LED_PWM_Update();
    
    // Update effects every 10 PWM cycles (reduce effect update frequency)
    static uint32_t effect_counter = 0;
    effect_counter++;
    if (effect_counter >= 10) {
        effect_counter = 0;
        RGB_LED_Effect_Update();
    }
}

/**
 * @brief RGB LED test function
 */
void RGB_LED_Test(void)
{
    printf("\r\n=== RGB LED Test Start ===\r\n");
    
    // Initialize RGB LED
    RGB_LED_Init();
    
    // Test various modes
    RGB_Mode_t test_modes[] = {
        RGB_MODE_STATIC_RED,
        RGB_MODE_STATIC_GREEN, 
        RGB_MODE_STATIC_BLUE,
        RGB_MODE_BLINK_RED,
        RGB_MODE_BREATHING,
        RGB_MODE_ALTERNATE
    };
    
    const char* mode_names[] = {
        "Static Red",
        "Static Green",
        "Static Blue", 
        "Red Blinking",
        "Breathing Effect",
        "Dual Alternating"
    };
    
    for (int i = 0; i < sizeof(test_modes)/sizeof(test_modes[0]); i++) {
        printf("Testing mode: %s\r\n", mode_names[i]);
        RGB_LED_SetMode(test_modes[i]);
        
        // Set color according to mode
        switch (test_modes[i]) {
            case RGB_MODE_STATIC_RED:
            case RGB_MODE_BLINK_RED:
                RGB_LED_SetColor(255, 0, 0);
                break;
            case RGB_MODE_STATIC_GREEN:
                RGB_LED_SetColor(0, 255, 0);
                break;
            case RGB_MODE_STATIC_BLUE:
                RGB_LED_SetColor(0, 0, 255);
                break;
            default:
                RGB_LED_SetColor(255, 255, 255);
                break;
        }
        
        // Run each mode for 3 seconds
        HAL_Delay(3000);
    }
    
    // Test completed, turn off LED
    RGB_LED_SetMode(RGB_MODE_OFF);
    printf("=== RGB LED Test Completed ===\r\n\r\n");
}

/**
 * @brief TIM3 interrupt callback function
 * @param htim: Timer handle
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        // Update RGB LED
        RGB_LED_Update();
    }
    
    // Keep original TIM1 callback
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
} 
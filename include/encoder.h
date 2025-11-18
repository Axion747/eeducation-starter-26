#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <Arduino.h>
#include <stdint.h>
#include "pin.h"

/**
 * @brief Simple rotary encoder abstraction with spin and button callbacks.
 * 
 * The encoder tracks quadrature input from two channels (A/B)
 * and a pushbutton input. Callbacks receive the encoder pointer so
 * users can easily access state or data from within their handlers.
 */
typedef struct encoder {
    pin_t pin_a;   ///< Channel A pin
    pin_t pin_b;   ///< Channel B pin
    pin_t pin_btn; ///< Button pin

    volatile int32_t position;  ///< Accumulated position
    volatile uint8_t last_state;///< Last AB state (00..11)

    void (*spin_cb)(struct encoder* enc, int32_t delta); ///< Called on rotation
    void (*button_cb)(struct encoder* enc);              ///< Called on button press
} encoder_t;

/**
 * @brief Initializes the encoder with quadrature and button pins.
 * 
 * @param enc Pointer to encoder instance
 * @param pin_a Channel A pin
 * @param pin_b Channel B pin
 * @param pin_btn Button pin
 */
void encoder_init(encoder_t* enc, pin_t pin_a, pin_t pin_b, pin_t pin_btn);

/**
 * @brief Assigns a callback for spin events.
 * 
 * @param enc Pointer to encoder instance
 * @param cb  Function called when the encoder rotates (receives ±1 delta)
 */
void encoder_set_spin_callback(encoder_t* enc, void (*cb)(encoder_t* enc, int32_t delta));

/**
 * @brief Assigns a callback for button press events.
 * 
 * @param enc Pointer to encoder instance
 * @param cb  Function called when the button is pressed
 */
void encoder_set_button_callback(encoder_t* enc, void (*cb)(encoder_t* enc));

/**
 * @brief Returns the current accumulated position count.
 * 
 * @param enc Pointer to encoder instance
 * @return Current encoder position
 */
int32_t encoder_get_position(const encoder_t* enc);

/**
 * @brief Sets the current accumulated position.
 * 
 * @param enc Pointer to encoder instance
 * @param pos New position value
 */
void encoder_set_position(encoder_t* enc, int32_t pos);

/**
 * @brief Attaches CHANGE interrupts to A/B pins and a RISING interrupt to the button pin.
 * 
 * @param enc Pointer to encoder instance
 */
void attach_encoder_interrupts(encoder_t* enc);

#endif  // __ENCODER_H__

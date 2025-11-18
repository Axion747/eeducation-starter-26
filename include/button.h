#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Arduino.h>
#include <stdint.h>

#include "pin.h"

/**
 * @brief Simple button abstraction with a single callback.
 */
typedef struct button {
    pin_t pin;
    void (*callback)(struct button* ctx);  // Function to call when button is pressed
    void* ctx;                    // User data passed to callback
    // ISR/debounce state (managed internally)
    volatile bool event_pending;  // set in ISR, cleared in button_process
    volatile uint8_t last_raw;    // raw level sampled in ISR
    volatile uint32_t last_debounce_ms; // millis() timestamp used by process
    uint32_t debounce_ms;         // debounce interval (ms)
    bool stable_state;            // debounced stable state (true == pressed)
} button_t;

/**
 * @brief Initializes the button.
 *
 * @param btn Pointer to button instance.
 * @param pin Input pin number.
 */
void button_init(button_t* btn, pin_t pin);

/**
 * @brief Assigns a callback function to the button.
 *
 * @param btn Pointer to button instance.
 * @param cb  Callback function pointer.
 * @param ctx User-provided context pointer.
 */
void button_set_callback(button_t* btn, void (*cb)(button_t* ctx), void* ctx);


/**
 * @brief Reads the current digital state of the button pin.
 *
 * @param btn Pointer to button instance.
 * @return uint8_t 0 or 1 depending on pin state.
 */
bool button_read(const button_t* btn);

/**
 * @brief Attaches an interrupt for the given button instance.
 * 
 * @param btn Pointer to button instance.
 * @param pin The input pin number.
 */
void attach_button_interrupt(button_t* btn, pin_t pin);

/**
 * @brief Process pending button events and run debounced callbacks.
 *
 * This must be called from the main loop or a task (not from ISR).
 */
void button_process(button_t* btn);


#endif  // __BUTTON_H__
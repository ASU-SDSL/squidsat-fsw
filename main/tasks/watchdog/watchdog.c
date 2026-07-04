#include "watchdog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "hardware/powman.h"

#include "HardwareConfig.h"

/// @brief Stores instance of this task, assigned in main.c and used for notifications (similar to binary semaphores)
TaskHandle_t xWatchdogTaskHandler;

// use watchdog_enable_caused_reboot() to tell if the internal watchdog rebooted
// the device

/// @brief Number of milliseconds before watchdog will reboot without watchdog_update
/// Should be less than the external - external right now I think is 3s
/// that should probably be longer
#define WATCHDOG_BUILT_IN_TIMEOUT_MS (2 * 1000) // 2s

/**
 * @brief Initialize watchdog task
 *
 */
void watchdog_init()
{
    // enable built in
    watchdog_enable(WATCHDOG_BUILT_IN_TIMEOUT_MS, true);

    // set up external heartbeat?
    gpio_init(WD_DONE_PIN);
    gpio_set_dir(WD_DONE_PIN, GPIO_OUT);
    gpio_put(WD_DONE_PIN, 0); // normally low
}

/**
 *  @brief Task that controls the resetting of the hardware watchdog - MAX706RESA - this module needs to be reset (GPIO toggled) within 1.6s with 100ns minimum pulse
 *  Any task can send this task a signal to stop reseting the watchdog, causing a simple processor reboot
 */
void watchdog_task(void *pvParameters)
{

    while (1)
    {
        // check for freeze
        if (ulTaskNotifyTake(pdTRUE, 0) > 0)
        {
            while (1)
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        // update internal
        watchdog_update();

        // toggle external - recognized as a low to high transition
        gpio_put(WD_DONE_PIN, 1); // go high
        // do we need a delay? DONE Pulse width must be at least 100 ns
        sleep_us(1);
        gpio_put(WD_DONE_PIN, 0); // go back low for next

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 *  @brief Runs the watchdog reset loop and will stop if given the freeze signal from the watchdog_freeze function
 */
void watchdog_freeze()
{
    xTaskNotifyGive(xWatchdogTaskHandler);
}

/**
 * @brief Checks POWMAN registers for flags indicating the last reset was caused
 * by a power event
 *
 * @return true
 * @return false
 */
bool watchdog_check_browned_out()
{
    uint32_t powman_chip_reset = powman_hw->chip_reset;

    return powman_chip_reset & 
        ((POWMAN_CHIP_RESET_HAD_BOR_BITS | POWMAN_CHIP_RESET_HAD_GLITCH_DETECT_BITS));
}
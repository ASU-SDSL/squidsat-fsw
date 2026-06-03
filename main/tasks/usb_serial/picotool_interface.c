
#include "tusb_config.h"
#include "tusb.h"

#include "pico/unique_id.h"
#include "pico/usb_reset_interface.h"
#include "pico/bootrom.h"
#include "device/usbd_pvt.h"
#include "hardware/watchdog.h"

// Reference: https://github.com/raspberrypi/pico-sdk/blob/a1438dff1d38bd9c65dbd693f0e5db4b9ae91779/src/rp2_common/pico_stdio_usb/reset_interface.c
// Reference: https://github.com/raspberrypi/pico-sdk/blob/a1438dff1d38bd9c65dbd693f0e5db4b9ae91779/src/rp2_common/pico_stdio_usb/include/pico/stdio_usb.h

// defines from stdio_usb.h

// Any modes disabled here can't be re-enabled by picotool via VENDOR_INTERFACE.
// PICO_CONFIG: PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK, Optionally disable either the mass storage interface (bit 0) or the PICOBOOT interface (bit 1) when entering BOOTSEL mode via USB (either VIA_BAUD_RATE or VIA_VENDOR_INTERFACE), type=int, min=0, max=3, default=0, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK
#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u
#endif

// PICO_CONFIG: PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS, Delay in ms before rebooting via regular flash boot, default=100, group=pico_stdio_usb
#ifndef PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS
#define PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS 100
#endif

// end

static uint8_t itf_num;

static void resetd_init(void) {
    
}

static void resetd_reset(uint8_t __unused rhport) {
    itf_num = 0;
}

static uint16_t resetd_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
              RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
              RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len, 0);

    itf_num = itf_desc->bInterfaceNumber;
    return drv_len;
}

// Support for parameterized reset via vendor interface control request
static bool resetd_control_xfer_cb(uint8_t __unused rhport, uint8_t stage, tusb_control_request_t const * request) {
    // nothing to do with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) return true;

    if (request->wIndex == itf_num) {

// #if PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_BOOTSEL
        if (request->bRequest == RESET_REQUEST_BOOTSEL) {
#ifdef PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED
            int gpio = PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED;
            bool active_low = PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED_ACTIVE_LOW;
#else
            int gpio = -1;
            bool active_low = false;
#endif
#if !PICO_STDIO_USB_RESET_BOOTSEL_FIXED_ACTIVITY_LED
            if (request->wValue & 0x100) {
                gpio = request->wValue >> 9u;
            }
            active_low = request->wValue & 0x200;
#endif
            rom_reset_usb_boot_extra(gpio, (request->wValue & 0x7f) | PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK, active_low);
            // does not return, otherwise we'd return true
        }
// #endif

// #if PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_RESET_TO_FLASH_BOOT
        if (request->bRequest == RESET_REQUEST_FLASH) {
            watchdog_reboot(0, 0, PICO_STDIO_USB_RESET_RESET_TO_FLASH_DELAY_MS);
            return true;
        }
// #endif

    }
    return false;
}

static bool resetd_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes) {
    return true;
}

static usbd_class_driver_t const _resetd_driver =
{
#if CFG_TUSB_DEBUG >= 2
    .name = "RESET",
#endif
    .init             = resetd_init,
    .reset            = resetd_reset,
    .open             = resetd_open,
    .control_xfer_cb  = resetd_control_xfer_cb,
    .xfer_cb          = resetd_xfer_cb,
    .sof              = NULL
};

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;

    return &_resetd_driver;
}
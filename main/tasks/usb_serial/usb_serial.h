#ifndef SERIAL
#define USB_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h> 

bool safe_tud_cdc_connected();

void usb_serial_init(); 
void usb_serial_task(void *params); 

extern TaskHandle_t usbTaskHandle;
extern SemaphoreHandle_t usb_mutex;

#endif // USB_H
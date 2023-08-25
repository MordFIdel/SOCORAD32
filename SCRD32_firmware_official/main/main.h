#ifndef MAIN_H
#define MAIN_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <errno.h>
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_gatt_common_api.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "eeprom.h"

#include "ui.h"
#include "uart.h"
#include "gpio.h"
#include "ble.h"

#define HIGH_LEVEL  false
#define LOW_LEVEL   true

#define WIDE_BAND	true
#define NARROW_BAND	false

#endif

#include "main.h"

#define TAG "MAIN"

SemaphoreHandle_t State_read_mutex = NULL;

void app_main(void)
{
	esp_err_t ret;

	/* Initialize NVS. */
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	State_read_mutex = xSemaphoreCreateMutex();
	xSemaphoreGive(State_read_mutex);

	if (beginEEPROM(4096) == false)
	{
		ESP_LOGE(TAG, "beginEEPROM fail");
	}

	ble_init();

	xTaskCreatePinnedToCore(uiTask, "uiTask", 4096, NULL, 3, NULL, 1);
	createUartTasks();
	xTaskCreatePinnedToCore(gpioTask, "gpioTask", 4096, NULL, 3, NULL, 1);
	
	while(1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

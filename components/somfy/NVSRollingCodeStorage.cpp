#include "NVSRollingCodeStorage.h"

#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "esphome/core/log.h"

NVSRollingCodeStorage::NVSRollingCodeStorage(const char *name, const char *key) : name(name), key(key) {}

uint16_t NVSRollingCodeStorage::nextCode() {
	uint16_t code;
	esp_err_t err;
	nvs_handle rcs_handle;

	// Initialize NVS
	err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	err = nvs_open(name, NVS_READWRITE, &rcs_handle);
	ESP_ERROR_CHECK(err);

	err = nvs_get_u16(rcs_handle, key, &code);
	switch (err) {
		case ESP_OK:
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			code = 1;
			break;
		default:
			ESP_LOGD("somfy.nvs", "Error reading!");
			ESP_LOGD("somfy.nvs", esp_err_to_name(err));
	}
	err = nvs_set_u16(rcs_handle, key, code + 1);
	ESP_LOGD("somfy.nvs", (err != ESP_OK) ? "nvs_set failed!" : "nvs_set done");
	err = nvs_commit(rcs_handle);
	ESP_LOGD("somfy.nvs", (err != ESP_OK) ? "nvs_commit failed!" : "nvs_commit done");
	return code;
}

#include "storage.h"

#include "fx_api.h"
#include "fx_stm32_sd_driver.h"
#include "main.h"
#include "time.h"

#define STORAGE_FILE_COUNT 		 4U
#define STORAGE_MOUNT_DELAY_MS   200U
#define STORAGE_MOUNT_RETRY_MS   100U
#define STORAGE_MOUNT_TIMEOUT_MS 2000U

struct StorageFile {
	FX_FILE fx_file;
	bool in_use;
	bool valid;
};

extern SD_HandleTypeDef hsd1;

static FX_MEDIA media;

static UCHAR media_cache[FX_STM32_SD_DEFAULT_SECTOR_SIZE]
	__attribute__((aligned(4)));

static StorageFile files[STORAGE_FILE_COUNT];

static bool media_mounted;
static bool card_present_prev;

static bool mount_pending;
static uint32_t mount_started_at;
static uint32_t mount_last_attempt_at;

static bool card_present(void)
{
	return HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin) == GPIO_PIN_RESET;
}

static void media_invalidate(void)
{
	for (size_t i = 0; i < STORAGE_FILE_COUNT; i++) {
		if (files[i].in_use) {
			files[i].valid = false;
		}
	}

	if (media_mounted) {
		(void)fx_media_abort(&media);
		(void)HAL_SD_DeInit(&hsd1);
	}

	media_mounted = false;
}

static Error media_mount(void)
{
	hsd1.Instance = SDMMC1;
	hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
	hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
	hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
	hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd1.Init.ClockDiv = 1;

	if (HAL_SD_Init(&hsd1) != HAL_OK) {
		(void)HAL_SD_DeInit(&hsd1);
		return ERROR_IO;
	}

	const UINT status = fx_media_open(
		&media,
		"SD",
		fx_stm32_sd_driver,
		NULL,
		media_cache,
		sizeof(media_cache)
	);

	if (status != FX_SUCCESS) {
		(void)HAL_SD_DeInit(&hsd1);
		return ERROR_IO;
	}

	media_mounted = true;
	return ERROR_NONE;
}

Error storage_init(void)
{
	card_present_prev = card_present();

	if (!card_present_prev) {
		return ERROR_UNAVAILABLE;
	}

	const Error error = media_mount();

	if (error != ERROR_NONE) {
		const uint32_t now = time_ms();

		mount_started_at = now;
		mount_last_attempt_at = now;
		mount_pending = true;
	}

	return error;
}

bool storage_available(void)
{
	return media_mounted;
}

void storage_process(void)
{
	const bool present = card_present();
	const uint32_t now = time_ms();

	if (present != card_present_prev) {
		card_present_prev = present;

		if (!present) {
			mount_pending = false;
			media_invalidate();
			return;
		}

		mount_started_at = now;
		mount_last_attempt_at = now;
		mount_pending = true;
		return;
	}

	if (!mount_pending) {
		return;
	}

	if (now - mount_started_at >= STORAGE_MOUNT_TIMEOUT_MS) {
		mount_pending = false;
		return;
	}

	if (now - mount_started_at < STORAGE_MOUNT_DELAY_MS) {
		return;
	}

	if (now - mount_last_attempt_at < STORAGE_MOUNT_RETRY_MS) {
		return;
	}

	if (media_mount() == ERROR_NONE) {
		mount_pending = false;
		return;
	}

	mount_last_attempt_at = time_ms();
}

Error storage_open(StorageFile **file, const char *path)
{
	StorageFile *slot = NULL;

	ASSERT(file != NULL);
	ASSERT(path != NULL);

	*file = NULL;

	if (!media_mounted) {
		return ERROR_UNAVAILABLE;
	}

	for (size_t i = 0; i < STORAGE_FILE_COUNT; i++) {
		if (!files[i].in_use) {
			slot = &files[i];
			break;
		}
	}

	if (slot == NULL) {
		return ERROR_BUSY;
	}

	const UINT status = fx_file_open(
		&media,
		&slot->fx_file,
		(CHAR *)path,
		FX_OPEN_FOR_READ
	);

	if (status == FX_NOT_FOUND) {
		return ERROR_NOT_FOUND;
	}

	if (
		status == FX_INVALID_NAME || 
		status == FX_INVALID_PATH ||
		status == FX_NOT_A_FILE
	)
	{
		return ERROR_INVALID_DATA;
	}

	if (status != FX_SUCCESS) {
		if (status == FX_IO_ERROR) {
			media_invalidate();
		}

		return ERROR_IO;
	}

	slot->in_use = true;
	slot->valid = true;
	*file = slot;

	return ERROR_NONE;
}

Error storage_read(
	StorageFile *file,
	void *buffer,
	const size_t capacity,
	size_t *bytes_read
)
{
	ULONG actual = 0;

	ASSERT(file != NULL);
	ASSERT(bytes_read != NULL);
	ASSERT(buffer != NULL || capacity == 0U);
	ASSERT(file->in_use);

	*bytes_read = 0;

	if (!file->valid || !media_mounted) {
		return ERROR_UNAVAILABLE;
	}

	if (capacity == 0U) {
		return ERROR_NONE;
	}

	const UINT status = fx_file_read(
		&file->fx_file,
		buffer,
		capacity,
		&actual
	);

	if (status != FX_SUCCESS && status != FX_END_OF_FILE) {
		if (status == FX_IO_ERROR) {
			media_invalidate();
		}

		return ERROR_IO;
	}

	*bytes_read = (size_t)actual;
	return ERROR_NONE;
}

Error storage_seek(StorageFile *file, const uint32_t offset)
{
	ASSERT(file != NULL);
	ASSERT(file->in_use);

	if (!file->valid || !media_mounted) {
		return ERROR_UNAVAILABLE;
	}

	const UINT status = fx_file_seek(&file->fx_file, offset);

	if (status != FX_SUCCESS) {
		if (status == FX_IO_ERROR) {
			media_invalidate();
		}

		return ERROR_IO;
	}

	return ERROR_NONE;
}

Error storage_close(StorageFile *file)
{
	ASSERT(file != NULL);
	ASSERT(file->in_use);

	if (!file->valid || !media_mounted) {
		file->in_use = false;
		file->valid = false;
		return ERROR_NONE;
	}

	const UINT status = fx_file_close(&file->fx_file);

	if (status != FX_SUCCESS) {
		if (status == FX_IO_ERROR) {
			media_invalidate();
			file->in_use = false;
			file->valid = false;
		}

		return ERROR_IO;
	}

	file->in_use = false;
	file->valid = false;

	return ERROR_NONE;
}

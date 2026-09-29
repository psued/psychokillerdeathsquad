#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "error.h"

typedef struct StorageFile StorageFile;

/**
 * Initialize storage and attempt to mount the SD card.
 */
Error storage_init(void);

/**
 * Return whether storage is currently available.
 */
bool storage_available(void);

/**
 * Process card insertion, removal, and recovery.
 */
void storage_process(void);

/**
 * Open an existing file for reading.
 * Set *file to NULL on failure.
 */
Error storage_open(StorageFile **file, const char *path);

/**
 * Read up to capacity bytes.
 * Return zero bytes at EOF.
 */
Error storage_read(
	StorageFile *file,
	void *buffer,
	size_t capacity,
	size_t *bytes_read
);

/**
 * Seek to an absolute byte offset.
 */
Error storage_seek(StorageFile *file, uint32_t offset);

/**
 * Close a file and release its handle.
 */
Error storage_close(StorageFile *file);

#endif

#pragma once

#include <cstddef>
#include <cstdint>

#include "io/buffer.h"
#include "util/nocopy.h"
#include "util/status.h"

/**
 * The `BufferManager` is a class that manages access to a disk device.
 *
 * The disk device is block addressed.  Buffers are accessed via the `get`
 * call.
 *
 * The `BufferManager` is not thread-safe.  Only one `BufferManager` may be
 * present for a given block device.
 */
class BufferManager {
    DISALLOW_COPY(BufferManager);
public:
    /**
     * Create a BufferManager for the device as `devicePath`.
     */
    BufferManager(const char* devicePath);
    ~BufferManager();

    /**
     * Get the buffer addressed at `blockNum`, reading it from disk.
     *
     * Returns: A successful status if the buffer was available, or a
     * NO_SPACE error if the block requested is outside the range of the
     * device.
     */
    StatusWith<Buffer> get(std::size_t blockNum) const;

    /**
     * Write the buffer back to disk.
     *
     * It is invalid to call write with a 'buffer' obtained from another
     * BufferManager.
     */
    Status write(const Buffer& buffer) const;

    // Get the block size in bytes.
    uint64_t getBlockSize() const;

    // Get the device size in blocks.
    uint64_t getDeviceSize() const;

private:
    // The file descriptor for the device.
    int _devFd;

    // The size of blocks on the device.
    const uint64_t _blockSize;

    // The number of blocks available on the device.
    std::size_t _deviceSize;
};

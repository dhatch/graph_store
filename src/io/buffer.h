#pragma once

#include <cstddef>
#include <cstdint>

#include "util/nocopy.h"

/**
 * Represents a region of disk space.  Obtainable through the `BufferManager`.
 */
class Buffer {
    DISALLOW_COPY(Buffer);
public:
    ~Buffer();
    Buffer(Buffer&& other);

    /**
     * Get access to the underlying memory region.
     */
    void* getRaw() const;

    /**
     * Get the size of the buffer.
     */
    std::size_t size() const;

    friend class BufferManager;
private:
    Buffer(std::size_t size, uint64_t blockNum);

    void *_data;
    std::size_t _size;
    uint64_t _blockNum;
};

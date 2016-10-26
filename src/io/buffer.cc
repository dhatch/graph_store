#include "io/buffer.h"

#include <sys/mman.h>

#include "util/assert.h"

#if defined(__APPLE__) && defined(__MACH__)
#define _MAP_ANONYMOUS MAP_ANON
#else
#define _MAP_ANONYMOUS MAP_ANONYMOUS
#endif

Buffer::Buffer(std::size_t size, uint64_t blockNum) :
        _data(nullptr), _size(size), _blockNum(blockNum) {
    _data = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | _MAP_ANONYMOUS, 0, 0);
    if (_data == MAP_FAILED) {
        check_errno(-1);
    }
}

Buffer::Buffer(Buffer&& other) : _data(other._data), _size(other._size),
                                       _blockNum(other._blockNum) {
    other._data = nullptr;
}

Buffer::~Buffer() {
    if (_data) {
        check_errno(munmap(_data, _size));
    }
}

void * Buffer::getRaw() const {
    return _data;
}

std::size_t Buffer::size() const {
    return _size;
}

#include "io/buffer_manager.h"

#include <unistd.h>
#include <fcntl.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/disk.h>
#else
#include <linux/fs.h>
#endif

#include <sys/ioctl.h>

#include "platform_params.h"
#include "util/assert.h"
#include "util/status.h"

BufferManager::BufferManager(const char* devicePath)
        : _devFd(0), _blockSize(Platform::BUFFER_BLOCK_SIZE) {
#if defined(__APPLE__) && defined(__MACH__)
    _devFd = open(devicePath, O_SYNC | O_RDWR);
    check_errno(_devFd);
    check_errno(fcntl(_devFd, F_NOCACHE, 1));
#else
    check_errno(_devFd);
    _devFd = open(devicePath, O_DIRECT | O_SYNC | O_RDWR);
#endif


#if defined(__APPLE__) && defined(__MACH__)
    uint64_t numBlocks;
    uint64_t blockSize;
    check_errno(ioctl(_devFd, DKIOCGETBLOCKCOUNT, &numBlocks));
    check_errno(ioctl(_devFd, DKIOCGETBLOCKSIZE, &blockSize));
    _deviceSize = numBlocks * blockSize / _blockSize;
#else
    uint64_t byteSize;
    check_errno(ioctl(_devFd, BLKGETSIZE64, &byteSize));
    _deviceSize = byteSize / _blockSize;
#endif
}

BufferManager::~BufferManager() {
    check_errno(close(_devFd));
}

StatusWith<Buffer> BufferManager::get(std::size_t blockNum, bool zeroed) const {
    if (blockNum >= _deviceSize) {
        return StatusCode::NO_SPACE;
    }

    Buffer buf(_blockSize, blockNum);
    if (!zeroed) {
        std::size_t read = pread(
            _devFd, buf.getRaw(), _blockSize, _blockSize * blockNum);

        check_errno((int)read);
        invariant(read == _blockSize);
    }

    return std::move(buf);
}

Status BufferManager::write(const Buffer& buffer) const {
    std::size_t written = pwrite(_devFd, buffer.getRaw(), _blockSize,
                                 _blockSize * buffer._blockNum);

    check_errno((int)written);
    invariant(written == _blockSize);

    return StatusCode::SUCCESS;
}

uint64_t BufferManager::getBlockSize() const {
    return _blockSize;
}

uint64_t BufferManager::getDeviceSize() const {
    return _deviceSize;
}

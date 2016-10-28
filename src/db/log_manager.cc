#include "db/log_manager.h"

#include "platform_params.h"
#include "util/stdx/memory.h"

#define BLOCK_MAGIC 1234567

uint64_t calculate_checksum(const void *start, const void *end) {
    uint64_t checksum = 0;
    const uint64_t *ptr = static_cast<const uint64_t*>(start);
    while (ptr < end) {
        checksum ^= *ptr;
        ptr++;
    }

    return checksum;
}

bool check_checksum(uint64_t checksum, const void *start, const void *end) {
    return checksum == calculate_checksum(start, end);
}

/**
 * A struct used for representing the memory layout of a superblock.
 */
struct SuperBlock {
    uint64_t checksum;
    uint32_t __magic;
    uint32_t generation;
    uint32_t logSegmentStart; // First valid log block of this generation.
    uint32_t logSegmentSize; // Number of blocks in the log.
    bool __END;

    // Call when initializing a new superblock.
    void init(uint64_t superblockAddress) {
        __magic = BLOCK_MAGIC;
        generation = 0;
        logSegmentStart = superblockAddress + 1;
        logSegmentSize = 0;
    }

    bool valid() const {
        return check_checksum(checksum, &__magic, &__END);
    };

    // Call before writing the superblock.
    void prewrite() {
        checksum = calculate_checksum(&__magic, &__END);
    }
};

constexpr std::size_t LOG_BLOCK_ENTRY_COUNT =
    (Platform::BUFFER_BLOCK_SIZE - 4*sizeof(uint64_t)) / sizeof(LogManager::Entry);

/**
 * Represents the memory layout of a log block.
 */
struct LogBlock {
    uint64_t checksum;
    uint64_t __magic;
    uint64_t generation;
    uint32_t nEntries;
    LogManager::Entry entries[LOG_BLOCK_ENTRY_COUNT];
    bool __END;

    void init(uint64_t generation) {
        __magic = BLOCK_MAGIC;
        generation = generation;
        nEntries = 0;
    }

    bool valid() const {
        return check_checksum(checksum, &__magic, &__END);
    }

    void prewrite() {
        checksum = calculate_checksum(&__magic, &__END);
    }
};

LogManager::LogManager(const BufferManager& manager,
                       std::pair<std::size_t, std::size_t> blockRange) :
        _bufferManager(manager), _logMinBlock(blockRange.first),
        _logMaxBlock(blockRange.second) {
    invariant(_logMaxBlock < manager.getDeviceSize());
    invariant(_logMaxBlock > _logMinBlock);
}

void LogManager::init() {
    auto status_with_buffer = _bufferManager.get(_logMinBlock);
    invariant(status_with_buffer);
    _superblock = stdx::make_unique<Buffer>(std::move(*status_with_buffer));

    SuperBlock *superblock = static_cast<SuperBlock*>(_superblock->getRaw());
    invariant(superblock->valid());

    auto status_with_current_buffer = _bufferManager.get(
            superblock->logSegmentStart + superblock->logSegmentSize - 1);
    invariant(status_with_current_buffer);
    _currentBlock = stdx::make_unique<Buffer>(std::move(*status_with_current_buffer));

    LogBlock *logBlock = static_cast<LogBlock*>(_currentBlock->getRaw());
    invariant(logBlock->valid());
}

void LogManager::format() {
    auto status_with_buffer = _bufferManager.get(_logMinBlock);
    invariant(status_with_buffer);
    _superblock = stdx::make_unique<Buffer>(std::move(*status_with_buffer));

    SuperBlock *superblock = static_cast<SuperBlock*>(_superblock->getRaw());
    if (!superblock->valid()) {
        superblock->init(_logMinBlock);
        superblock->prewrite();
        _bufferManager.write(*_superblock);
    }

    increaseGeneration();
}

Status LogManager::logOperation(LogManager::Entry entry) {
    LogBlock *logBlock = static_cast<LogBlock*>(_currentBlock->getRaw());

    // Block is full.
    if (logBlock->nEntries == LOG_BLOCK_ENTRY_COUNT) {
        // Create new block.
        SuperBlock *superblock = static_cast<SuperBlock*>(_superblock->getRaw());

        auto status_with_new_block = _bufferManager.get(
               superblock->logSegmentStart + superblock->logSegmentSize);
        if (!status_with_new_block) {
            return status_with_new_block;
        }

        _currentBlock = stdx::make_unique<Buffer>(std::move(*status_with_new_block));
        logBlock = static_cast<LogBlock*>(_currentBlock->getRaw());
        logBlock->init(superblock->generation);

        superblock->logSegmentSize++;
        superblock->prewrite();
        _bufferManager.write(*_superblock);
    }

    logBlock->entries[logBlock->nEntries++] = entry;
    logBlock->prewrite();
    _bufferManager.write(*_currentBlock);

    return StatusCode::SUCCESS;
}

uint64_t LogManager::increaseGeneration() {
    SuperBlock *superblock = static_cast<SuperBlock*>(_superblock->getRaw());

    superblock->generation++;
    superblock->logSegmentStart = _logMinBlock + 1;
    superblock->logSegmentSize = 1;

    superblock->prewrite();
    _bufferManager.write(*_superblock);

    auto status_with_buffer = _bufferManager.get(
            superblock->logSegmentStart);
    invariant(status_with_buffer);
    _currentBlock = stdx::make_unique<Buffer>(std::move(*status_with_buffer));

    LogBlock *logBlock = static_cast<LogBlock*>(_currentBlock->getRaw());
    logBlock->init(superblock->generation);
    logBlock->prewrite();

    _bufferManager.write(*_currentBlock);

    return superblock->generation;
}

uint64_t LogManager::getGeneration() const {
    SuperBlock *superblock = static_cast<SuperBlock*>(_superblock->getRaw());
    return superblock->generation;
}

LogManager::Reader::Reader(LogManager& logManager) : _logManager(logManager) {
    SuperBlock *superblock = static_cast<SuperBlock*>(_logManager._superblock->getRaw());
    _startBlock = superblock->logSegmentStart;
    _endBlock = _startBlock + superblock->logSegmentSize;
    _generation = superblock->generation;
    _blockNum = _startBlock;

    if (_startBlock < _endBlock) {
        auto status_with_buffer = _logManager._bufferManager.get(_startBlock);
        invariant(status_with_buffer);
        _buffer = stdx::make_unique<Buffer>(std::move(*status_with_buffer));
        invariant(static_cast<LogBlock*>(_buffer->getRaw())->valid());
    }
}

bool LogManager::Reader::hasNext() const {
    if (!_buffer)
        return false;

    if (_blockNum < _endBlock - 1)
        return true;

    LogBlock* logBlock = static_cast<LogBlock*>(_buffer->getRaw());
    return _entryIndex < logBlock->nEntries;
}

LogManager::Entry LogManager::Reader::getNext() {
    invariant(_buffer);
    LogBlock* logBlock = static_cast<LogBlock*>(_buffer->getRaw());
    invariant(logBlock->valid());

    // At end of current buffer.
    if (_entryIndex == logBlock->nEntries) {
        _blockNum++;
        _entryIndex = 0;
        invariant(_blockNum < _endBlock);

        auto status_with_next_buffer = _logManager._bufferManager.get(_blockNum);
        invariant(status_with_next_buffer);
        _buffer = stdx::make_unique<Buffer>(std::move(*status_with_next_buffer));

        logBlock = static_cast<LogBlock*>(_buffer->getRaw());
        invariant(logBlock->valid());
    }

    invariant(_entryIndex < LOG_BLOCK_ENTRY_COUNT);
    LogManager::Entry entry = logBlock->entries[_entryIndex];

    _entryIndex++;
    return entry;
}

void LogManager::Reader::close() {
    _logManager.releaseReader();
}

LogManager::Reader& LogManager::readLog() {
    _reader.reset(new Reader(*this));
    return *_reader;
}

void LogManager::releaseReader() {
    _reader = nullptr;
}

std::ostream& operator<<(std::ostream& os, const LogManager::Entry& entry) {
    const char *str;
    switch (entry.opcode) {
        case LogManager::OpCode::ADD_NODE:
            str = "ADD_NODE";
            break;
        case LogManager::OpCode::REMOVE_NODE:
            str = "REMOVE_NODE";
            break;
        case LogManager::OpCode::ADD_EDGE:
            str = "ADD_EDGE";
            break;
        case LogManager::OpCode::REMOVE_EDGE:
            str = "REMOVE_EDGE";
            break;
    }

    os << "Entry(" << str << ", " << entry.idA << ", " << entry.idB << ")";
    return os;
}

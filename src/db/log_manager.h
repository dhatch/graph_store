#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>

#include "io/buffer_manager.h"
#include "io/buffer.h"
#include "util/status.h"

/**
 * Manages the on-disk log.
 *
 * Not thread-safe.
 */
class LogManager {
public:
    enum class OpCode : uint32_t {
        ADD_NODE,
        ADD_EDGE,
        REMOVE_NODE,
        REMOVE_EDGE
    };

    /**
     * Represents a log entry.
     */
    struct Entry {
        Entry(OpCode opcode, uint64_t idA, uint64_t idB) :
            opcode(opcode), idA(idA), idB(idB) {};

        bool operator==(const Entry& rhs) const {
            return opcode == rhs.opcode && idA == rhs.idA && idB == rhs.idB;
        }

        OpCode opcode;
        uint64_t idA;
        uint64_t idB;

        friend std::ostream& operator<<(std::ostream& os, const Entry& entry);
    };

    /**
     * Used to read the log sequentially.
     */
    class Reader {
    public:
        Entry getNext();
        bool hasNext() const;
        void close();

        friend class LogManager;
    private:
        Reader(LogManager& logManager);

        LogManager& _logManager;
        std::unique_ptr<Buffer> _buffer = nullptr;

        uint64_t _startBlock;
        uint64_t _endBlock;
        uint64_t _blockNum;
        uint64_t _generation;
        uint32_t _entryIndex = 0;
    };

    /**
     * Initialize a log manager from the block range of the associated buffer
     * manager.
     *
     * Call init() or format() before interacting with the log.
     */
    LogManager(const BufferManager& manager,
               std::pair<std::size_t, std::size_t> blockRange);

    /**
     * Read the log on a normal startup.
     */
    void init();

    /**
     * Format the log to an empty log.
     */
    void format();

    /**
     * Add an operation to the end of the log.
     *
     * Returns: NO_SPACE if the log is full, or success.
     */
    Status logOperation(Entry entry);

    /**
     * Increment the log generation.
     */
    uint64_t increaseGeneration();

    /**
     * Get the current log generation.
     */
    uint64_t getGeneration() const;

    /**
     * Return a log reader starting at the front of the log.
     *
     * It is invalid to request another log reader when one is already
     * open.
     *
     * It is invalid to perform any operation on the LogManager while a reader
     * is open.
     */
    Reader& readLog();
private:
    // Release our reader.  Invalidates all external references to the
    // reader.
    void releaseReader();

    // The buffer manager used to access the log.  Outlives the log manager.
    const BufferManager& _bufferManager;
    // The minimum block used by the log.
    const std::size_t _logMinBlock;
    // The maximum block used by the log.
    const std::size_t _logMaxBlock;

    // The log superblock.
    std::unique_ptr<Buffer> _superblock = nullptr;
    // The currently in use buffer.
    std::unique_ptr<Buffer> _currentBlock = nullptr;

    // The currently open log reader, if one exists.
    std::unique_ptr<Reader> _reader = nullptr;
};

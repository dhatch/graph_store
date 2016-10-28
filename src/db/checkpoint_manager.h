#pragma once

#include <utility>

#include "io/buffer_manager.h"
#include "util/nocopy.h"
#include "util/status.h"
#include "util/stdx/memory.h"

class LoggedStore;

class CheckpointManager {
    DISALLOW_COPY(CheckpointManager);
public:
    CheckpointManager(const BufferManager& bufferManager,
                      std::pair<uint64_t, uint64_t> blockRange,
                      LoggedStore* loggedStore);

    /**
     * Initialize the checkpoint storage.
     */
    void init();

    /**
     * Erase any checkpoint data.
     */
    void format();

    /**
     * Check whether a checkpoint exists with 'generationNumber'.
     */
    bool hasCheckpoint(uint64_t generationNumber);

    /*
     * Perform a checkpoint of the store, with 'generationNumber'
     */
    Status performCheckpoint(uint64_t generationNumber);

    /*
     * Restore a checkpoing of the store, with 'generationNumber'.
     *
     * Does nothing if the checkpoint on disk doesn't match
     * the generation number.
     */
    void restoreCheckpoint(uint64_t generationNumber);

private:
    const BufferManager& _bufferManager;
    LoggedStore* _loggedStore = nullptr;

    std::unique_ptr<Buffer> _superblock = nullptr;

    uint64_t _checkpointBlockMin;
    uint64_t _checkpointBlockMax;
};

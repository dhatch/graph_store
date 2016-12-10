#include "db/checkpoint_manager.h"

#include <exception>
#include <utility>

#include "db/logged_store.h"
#include "db/types.h"
#include "io/buffer_manager.h"
#include "util/assert.h"
#include "util/status.h"
#include "util/stdx/memory.h"
#include "platform_params.h"

struct CheckpointSuperBlock {
    bool checkpointed;
    uint64_t checkpointVersion;
    uint64_t nodeCount;
};

CheckpointManager::CheckpointManager(const BufferManager& bufferManager,
                                     std::pair<uint64_t, uint64_t> blockRange,
                                     LoggedStore* loggedStore)
        : _bufferManager(bufferManager), _loggedStore(loggedStore),
          _checkpointBlockMin(blockRange.first),
          _checkpointBlockMax(blockRange.second) {
    invariant(_checkpointBlockMin < _checkpointBlockMax);
}


void CheckpointManager::init() {
    auto status_with_buffer = _bufferManager.get(_checkpointBlockMin);
    invariant(status_with_buffer);
    _superblock = stdx::make_unique<Buffer>(std::move(*status_with_buffer));
}

void CheckpointManager::format() {
    auto status_with_buffer = _bufferManager.get(_checkpointBlockMin, true);
    invariant(status_with_buffer);
    _superblock = stdx::make_unique<Buffer>(std::move(*status_with_buffer));
    _bufferManager.write(*_superblock);
}

bool CheckpointManager::hasCheckpoint(uint64_t generationNumber) {
    invariant(_superblock);
    CheckpointSuperBlock* superblock = static_cast<CheckpointSuperBlock*>(_superblock->getRaw());

    return superblock->checkpointed && superblock->checkpointVersion == generationNumber;
}

class BlockWriter {
public:
    class OutOfSpaceException : public std::exception {
    };

    BlockWriter(const BufferManager& bufferManager, uint64_t start, uint64_t end)
            : _bufferManager(bufferManager), _start(start), _end(end),
              _currentBlock(_start),
              _current(std::move(*_bufferManager.get(_start))) {
        readBlock(_currentBlock);
    }

    void writeUint64(uint64_t data) {
        if (_currentBlock >= _end)
            throw OutOfSpaceException{};

        if (_currWrite >= _endWrite) {
            _currentBlock++;
            if (_currentBlock == _end)
                throw OutOfSpaceException{};

            readBlock(_currentBlock);
        }

        *(_currWrite++) = data;
    }

    void flush() {
        _bufferManager.write(_current);
    }

private:
    void readBlock(uint64_t blockNo) {
        _bufferManager.write(_current);
        _current = std::move(*_bufferManager.get(blockNo, true));

        _currWrite = static_cast<uint64_t *>(_current.getRaw());
        _endWrite = _currWrite + (_current.size() / sizeof(uint64_t));
    }

    const BufferManager& _bufferManager;
    uint64_t _start;
    uint64_t _end;
    uint64_t _currentBlock;

    Buffer _current;

    uint64_t* _currWrite;
    uint64_t* _endWrite;
};

Status CheckpointManager::performCheckpoint(uint64_t generationNumber) {
    invariant(_superblock);
    CheckpointSuperBlock* superblock = static_cast<CheckpointSuperBlock*>(_superblock->getRaw());

    // Update superblock.
    superblock->checkpointVersion = generationNumber;
    superblock->checkpointed = false;
    superblock->nodeCount = _loggedStore->_memoryStore._nodes.size();
    _bufferManager.write(*_superblock);

    // Write out node by node checkpoints.
    auto nodeIterator = _loggedStore->_memoryStore._nodes.begin();
    BlockWriter writer(_bufferManager, _checkpointBlockMin + 1, _checkpointBlockMax);
    try {
        while (nodeIterator != _loggedStore->_memoryStore._nodes.end()) {
            writer.writeUint64(nodeIterator->first);
            auto edges = nodeIterator->second->edges();
            writer.writeUint64(edges.size());
            for (const NodeId& edgeId : nodeIterator->second->edges()) {
                writer.writeUint64(edgeId);
            }

            nodeIterator++;
        }
    } catch (const BlockWriter::OutOfSpaceException&) {
        return StatusCode::NO_SPACE;
    }

    writer.flush();

    superblock->checkpointed = true;
    _bufferManager.write(*_superblock);

    return StatusCode::SUCCESS;
}

class BlockReader {
public:
    BlockReader(const BufferManager& bufferManager, uint64_t start, uint64_t end)
            : _bufferManager(bufferManager), _start(start), _end(end),
              _currentBlock(_start),
              _current(std::move(*_bufferManager.get(_start))) {
        readBlock(_currentBlock);
    }

    uint64_t readUint64() {
        invariant(_currentBlock < _end);

        if (_currRead >= _endRead) {
            _currentBlock++;
            readBlock(_currentBlock);
        }

        return *(_currRead++);
    }

private:
    void readBlock(uint64_t blockNo) {
        _current = std::move(*_bufferManager.get(blockNo));

        _currRead = static_cast<uint64_t *>(_current.getRaw());
        _endRead = _currRead + (_current.size() / sizeof(uint64_t));
    }

    const BufferManager& _bufferManager;
    uint64_t _start;
    uint64_t _end;
    uint64_t _currentBlock;

    Buffer _current;

    uint64_t* _currRead;
    uint64_t* _endRead;
};

void CheckpointManager::restoreCheckpoint(uint64_t generationNumber) {
    invariant(_superblock);
    CheckpointSuperBlock* superblock = static_cast<CheckpointSuperBlock*>(_superblock->getRaw());

    // Update superblock.
    if (superblock->checkpointVersion != generationNumber)
        return;

    // Write out node by node checkpoints.
    auto& memoryStore = _loggedStore->_memoryStore;
    auto nodesRemaining = superblock->nodeCount;

    BlockReader reader(_bufferManager, _checkpointBlockMin + 1,
                       _checkpointBlockMax);
    while (nodesRemaining--) {
        uint64_t nodeId = reader.readUint64();
        uint64_t edgeCounts = reader.readUint64();
        memoryStore.addNode(nodeId);
        Node *node = *memoryStore.findNode(nodeId);

        // Read edges
        while (edgeCounts--) {
            uint64_t edgeId = reader.readUint64();
            auto status_with_node = memoryStore.findNode(edgeId);
            if (!status_with_node) {
                memoryStore.addNode(edgeId);
                status_with_node = memoryStore.findNode(edgeId);
            }

            node->addEdge((*status_with_node)->getId());
        }
    }
}

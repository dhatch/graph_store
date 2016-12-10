#include "db/logged_store.h"

#include <iostream>
#include <mutex>

#include "db/log_manager.h"
#include "db/memory_store.h"
#include "db/types.h"
#include "util/status.h"

LoggedStore::LoggedStore(const char* deviceName, bool formatLog) :
        _bufferManager(deviceName),
        _log(_bufferManager, {0, _bufferManager.getDeviceSize() / 5}),
        _checkpoint(_bufferManager,
                    {_bufferManager.getDeviceSize() / 5, _bufferManager.getDeviceSize()},
                    this) {
    if (formatLog) {
        _log.format();
        _checkpoint.format();
    } else {
        _log.init();
        _checkpoint.init();
        recover();
    }
}

Status LoggedStore::addNode(NodeId nodeId) {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    Status status = _log.logOperation({LogManager::OpCode::ADD_NODE, nodeId, 0});
    if (!status)
        return status;

    return _memoryStore.addNode(nodeId);
}

Status LoggedStore::removeNode(NodeId nodeId) {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    Status status = _log.logOperation({LogManager::OpCode::REMOVE_NODE, nodeId, 0});
    if (!status)
        return status;

    return _memoryStore.removeNode(nodeId);
}

StatusWith<Node*> LoggedStore::findNode(NodeId nodeId) const {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    return _memoryStore.findNode(nodeId);
}

StatusWith<std::pair<Node*, Node*>> LoggedStore::getEdge(NodeId nodeAId,
                                                         NodeId nodeBId) const {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    return _memoryStore.getEdge(nodeAId, nodeBId);
}

Status LoggedStore::addEdge(NodeId nodeAId, NodeId nodeBId) {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    Status status = _log.logOperation({LogManager::OpCode::ADD_EDGE, nodeAId, nodeBId});
    if (!status)
        return status;

    return _memoryStore.addEdge(nodeAId, nodeBId);
}

Status LoggedStore::removeEdge(NodeId nodeAId, NodeId nodeBId) {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    Status status = _log.logOperation({LogManager::OpCode::REMOVE_EDGE, nodeAId, nodeBId});
    if (!status)
        return status;

    return _memoryStore.removeEdge(nodeAId, nodeBId);
}

StatusWith<NodeIdList> LoggedStore::getNeighbors(NodeId nodeId) const {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    return _memoryStore.getNeighbors(nodeId);
}

StatusWith<uint64_t> LoggedStore::shortestPath(NodeId nodeAId,
                                  NodeId nodeBId) const {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    return _memoryStore.shortestPath(nodeAId, nodeBId);
}

Status LoggedStore::checkpoint() {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto status = _checkpoint.performCheckpoint(_log.getGeneration());
    if (!status) {
        // Note the database is not recoverable at this point.
        return status;
    }

    _log.increaseGeneration();
    return StatusCode::SUCCESS;
}

void LoggedStore::recover() {
    std::lock_guard<std::recursive_mutex> guard(_lock);
    _checkpoint.restoreCheckpoint(_log.getGeneration() - 1);
    LogManager::Reader &reader = _log.readLog();
    while (reader.hasNext()) {
        LogManager::Entry entry = reader.getNext();
        switch (entry.opcode) {
            case LogManager::OpCode::ADD_NODE:
                _memoryStore.addNode(entry.idA);
                break;
            case LogManager::OpCode::REMOVE_NODE:
                _memoryStore.removeNode(entry.idA);
                break;
            case LogManager::OpCode::ADD_EDGE:
                _memoryStore.addEdge(entry.idA, entry.idB);
                break;
            case LogManager::OpCode::REMOVE_EDGE:
                _memoryStore.removeEdge(entry.idA, entry.idB);
                break;
        }
    }

    reader.close();
}

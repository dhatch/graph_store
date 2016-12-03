#include "db/replication_manager.h"

#include "util/assert.h"

#include "net/replication_outbound.h"

ReplicationManager::ReplicationManager(ReplicationManager::NodeType type,
                                    GraphStore* store,
                                    ReplicationOutbound *successorReplicator)
        : _type(type), _store(store), _successorReplicator(successorReplicator) {
    invariant(_store);
    if (_type != ReplicationManager::NodeType::TAIL) {
        invariant(_successorReplicator);
    }
}


bool ReplicationManager::replicateAddNode(NodeId nodeId) {
    if (_type == ReplicationManager::NodeType::TAIL)
        return true;

    // TODO handle timeout.

    return _successorReplicator->addNode(nodeId);
}

bool ReplicationManager::replicateRemoveNode(NodeId nodeId) {
    if (_type == ReplicationManager::NodeType::TAIL)
        return true;

    return _successorReplicator->removeNode(nodeId);
}

bool ReplicationManager::replicateAddEdge(NodeId nodeAId, NodeId nodeBId) {
    if (_type == ReplicationManager::NodeType::TAIL)
        return true;

    return _successorReplicator->addEdge(nodeAId, nodeBId);
}

bool ReplicationManager::replicateRemoveEdge(NodeId nodeAId, NodeId nodeBId) {
    if (_type == ReplicationManager::NodeType::TAIL)
        return true;

    return _successorReplicator->removeEdge(nodeAId, nodeBId);
}

ReplicationManager::ReplicatedCheckpointStatus ReplicationManager::replicateCheckpoint() {
    if (_type == ReplicationManager::NodeType::TAIL) {
        return ReplicationManager::ReplicatedCheckpointStatus::SUCCESS;
    }

    auto result = _successorReplicator->checkpoint();
    switch (result) {
        case ReplicationCheckpointResult::OUT_OF_SPACE:
            return ReplicatedCheckpointStatus::OUT_OF_SPACE;
        case ReplicationCheckpointResult::CHECKPOINT_DISABLED:
            return ReplicatedCheckpointStatus::CHECKPOINT_DISABLED;
        case ReplicationCheckpointResult::SUCCESS:
            return ReplicatedCheckpointStatus::SUCCESS;
        default:
            return ReplicatedCheckpointStatus::REPL_FAIL;
    }
}


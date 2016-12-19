#include "db/partition/partition_manager.h"

namespace {

std::pair<std::string, int> getAddrParts(const std::string& address) {
    auto pos = address.find_last_of(':');
    if (pos == std::string::npos) {
        return {};
    }

    return {address.substr(0, pos), std::atoi(address.substr(pos+1).c_str())};
}

}


PartitionManager::PartitionManager(PartitionConfig config)
    : _config(config) {
}

void PartitionManager::start() {
    if (_started) {
        return;
    }

    for (const auto& partition : _config.partitions()) {
        if (partition != _config.partitions()[_config.us()]) {
            std::pair<std::string, int> parts = getAddrParts(partition);
            _partitions.emplace_back(parts.first, parts.second);
        }
    }

    _started = true;
}

StatusWith<std::pair<NodeId, NodeId>> PartitionManager::getEdgePart(NodeId nodeA, NodeId nodeB) const {
    if (_config.target(nodeA) == _config.us()) {
        return {{nodeA, nodeB}};
    } else if (_config.target(nodeB) == _config.us()) {
        return {{nodeB, nodeA}};
    } else {
        return StatusCode::WRONG_PARTITION;
    }
}

Status PartitionManager::addEdge(NodeId nodeLocalId, NodeId nodeRemoteId) {
    invariant(_config.target(nodeLocalId) == _config.us());
    start();

    bool success = getPartitionOutbound(_config.target(nodeRemoteId)).addEdgePart(nodeRemoteId, nodeLocalId);
    if (success) {
        return StatusCode::SUCCESS;
    } else {
        return StatusCode::PARTITION_FAIL;
    }
}

Status PartitionManager::removeEdge(NodeId nodeLocalId, NodeId nodeRemoteId) {
    invariant(_config.target(nodeLocalId) == _config.us());
    start();

    bool success = getPartitionOutbound(_config.target(nodeRemoteId)).removeEdgePart(nodeRemoteId, nodeLocalId);
    if (success) {
        return StatusCode::SUCCESS;
    } else {
        return StatusCode::PARTITION_FAIL;
    }
}

PartitionOutbound& PartitionManager::getPartitionOutbound(uint64_t nodeId) {
    invariant(nodeId != _config.us());
    return _partitions[nodeId > _config.us() ? nodeId - 1 : nodeId];
}

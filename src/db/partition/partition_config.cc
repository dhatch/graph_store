#include "db/partition/partition_config.h"

#include "util/assert.h"

PartitionConfig::PartitionConfig(std::vector<std::string> partitions, uint64_t ourId)
  : _partitions(partitions), _id(ourId) {
}

uint64_t PartitionConfig::target(NodeId nodeId) const {
    return nodeId % _partitions.size();
}

uint64_t PartitionConfig::us() const {
    return _id;
}

const std::vector<std::string>& PartitionConfig::partitions() const {
    return _partitions;
}

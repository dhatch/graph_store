#pragma once

#include <string>
#include <vector>

#include "db/types.h"

/**
 * 'PartitionConfig' is responsible for targeting the appropriate partition.
 */
class PartitionConfig {
public:
    /**
     * Initalize with a list of partition IPs.
     */
    PartitionConfig(std::vector<std::string> partitions, uint64_t ourId);

    /**
     * Returns the id of the node upon which a node with 'nodeId' should reside.
     */
    uint64_t target(NodeId nodeId) const;

    /**
     * Get the listen IP and port of this server's partition.
     */
    uint64_t us() const;
    const std::vector<std::string>& partitions() const;
private:
    std::vector<std::string> _partitions;
    uint64_t _id;
};

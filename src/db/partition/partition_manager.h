#ifndef _DB_PARTITION_PARTITION_MANGER_H
#define _DB_PARTITION_PARTITION_MANGER_H

#include "util/nocopy.h"
#include "db/partition/partition_config.h"
#include "db/graph_store.h"

#include "net/partition_outbound.h"

/**
 * The partition manager is responsible for taking the appropriate
 * actions to partition data on other nodes.
 */
class PartitionManager {
    DISALLOW_COPY(PartitionManager);
public:
    /**
     * Create a partition manager.
     */
    PartitionManager(PartitionConfig config);

    /**
     * Get the edge part that resides on this partition, and a pair of local, remote,
     * or return WRONG_PARTITION.
     */
    StatusWith<std::pair<NodeId, NodeId>> getEdgePart(NodeId nodeA, NodeId nodeB) const;

    /**
     * Add an edge on other partitions.
     */
    Status addEdge(NodeId nodeLocalId, NodeId nodeRemoteId);

    /**
     * Remove an edge on other partitions.
     */
    Status removeEdge(NodeId nodeLocalId, NodeId nodeRemoteId);

private:
    PartitionOutbound& getPartitionOutbound(uint64_t id);
    void start();

    PartitionConfig _config;
    std::vector<PartitionOutbound> _partitions;

    bool _started = false;
};

#endif /* _DB_PARTITION_PARTITION_MANGER_H */

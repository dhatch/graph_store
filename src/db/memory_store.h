/**
 * memory_store.h: Store graph data in-memory.
 */

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "db/graph_store.h"
#include "db/types.h"
#include "util/status.h"
#include "util/nocopy.h"

/**
 * Graph store interface.
 */
class MemoryStore : public GraphStore {
    DISALLOW_COPY(MemoryStore);
public:
    MemoryStore() = default;

    /**
     * Add a node with id `node_id` to the store.
     */
    virtual Status addNode(NodeId nodeId) override;

    /**
     * Remove a node from the store.
     */
    virtual Status removeNode(NodeId nodeId) override;

    /**
     * Find a node in the store.
     */
    virtual StatusWith<Node*> findNode(NodeId nodeId) const override;

    /**
     * Find an edge in the store.
     */
    virtual StatusWith<std::pair<Node*, Node*>> getEdge(NodeId nodeAId,
                                                NodeId nodeBId) const override;

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    virtual Status addEdge(NodeId nodeAId, NodeId nodeBId) override;

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    virtual Status removeEdge(NodeId nodeAId, NodeId nodeBId) override;

    /**
     * Get an edge part between 'nodeLocalId' and 'nodeRemoteId'.
     */
    virtual Status getEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) const override;

    /**
     * Adds an edge part between `nodeLocalId` and `nodeRemoteId`.
     */
    virtual Status addEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) override;

    /**
     * Remove an edge part between 'nodeLocalId' and 'nodeRemoteId'.
     *
     * Precondition: 'nodeLocalId' is on this partition.
     */
    virtual Status removeEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) override;

    /**
     * Find the neighbors of 'nodeId'.
     */
    virtual StatusWith<NodeIdList> getNeighbors(NodeId nodeId) const override;

    /**
     * Find the length of the shortest path between 'nodeAId' and 'nodeBId'.
     *
     * Note, since there is no maximum distance, this implementation is
     * unbounded in its time and memory usage.
     */
    virtual StatusWith<uint64_t> shortestPath(NodeId nodeAId,
                                              NodeId nodeBId) const override;

    friend class CheckpointManager;
private:
    std::map<NodeId, std::unique_ptr<Node>> _nodes;

    mutable std::recursive_mutex _memoryStoreMutex;
};

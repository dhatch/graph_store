/**
 * memory_store.h: Store graph data in-memory.
 */

#pragma once

#include <map>
#include <memory>
#include <utility>

#include "db/types.h"
#include "util/status.h"
#include "util/nocopy.h"

/**
 * Graph store interface.
 */
class MemoryStore {
protected:
    DISALLOW_COPY(MemoryStore);
public:
    MemoryStore() = default;

    /**
     * Add a node with id `node_id` to the store.
     */
    Status addNode(NodeId nodeId);

    /**
     * Remove a node from the store.
     */
    Status removeNode(NodeId nodeId);

    /**
     * Find a node in the store.
     */
    StatusWith<Node*> findNode(NodeId nodeId) const;

    /**
     * Find an edge in the store.
     */
    StatusWith<std::pair<Node*, Node*>> getEdge(NodeId nodeAId,
                                                NodeId nodeBId) const;

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    Status addEdge(NodeId nodeAId, NodeId nodeBId);

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    Status removeEdge(NodeId nodeAId, NodeId nodeBId);

    /**
     * Find the neighbors of 'nodeId'.
     */
    StatusWith<NodeIdList> getNeighbors(NodeId nodeId) const;

    /**
     * Find the length of the shortest path between 'nodeAId' and 'nodeBId'.
     *
     * Note, since there is no maximum distance, this implementation is
     * unbounded in its time and memory usage.
     */
    StatusWith<uint64_t> shortestPath(NodeId nodeAId, NodeId nodeBId) const;

private:
    std::map<NodeId, std::unique_ptr<Node>> _nodes;
};

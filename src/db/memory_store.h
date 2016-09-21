/**
 * memory_store.h: Store graph data in-memory.
 */

#pragma once

#include "src/db/types.h"
#include "src/util/status.h"
#include "src/util/nocopy.h"

/**
 * Graph store interface.
 */
class MemoryStore {
protected:
    DISALLOW_COPY(MemoryStore);
public:
    /**
     * Add a node with id `node_id` to the store.
     */
    Status addNode(NodeId nodeId);

    /**
     * Remove a node from the store.
     */
    Status removeNode(NodeId nodeId);

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    Status addEdge(NodeId nodeAId, NodeId nodeBId);

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    Status removeEdge(NodeId nodeAId, NodeId nodeBId);

    /**
     * Find a node in the store.
     */
    Status findNode(NodeId nodeId) const;

    /**
     * Find an edge in the store.
     */
    Status getEdge(NodeId nodeAId, NodeId nodeBId) const;

    /**
     * Find the neighbors of 'nodeId'.
     */
    StatusWith<NodeList> getNeighbors(NodeId nodeId) const;

    /**
     * Find the shortest path between 'nodeAId' and 'nodeBId'.
     */
    StatusWith<NodeList> shortestPath(NodeId nodeAId, NodeId nodeBId) const;

private:
    std::map<NodeId, Node> _nodes;
};

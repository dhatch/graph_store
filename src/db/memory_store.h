/**
 * memory_store.h: Store graph data in-memory.
 */

#pragma once

#include <cstdint>

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
    Status addNode(uint64_t nodeId);

    /**
     * Remove a node from the store.
     */
    Status removeNode(uint64_t nodeId);

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    Status addEdge(uint64_t nodeAId, uint64_t nodeBId);

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    Status removeEdge(uint64_t nodeAId, uint64_t nodeBId);

    /**
     * Find a node in the store.
     */
    Status findNode(uint64_t nodeId);

    /**
     * Find an edge in the store.
     */
    Status getEdge(uint64_t nodeAId, uint64_t nodeBId);

    /**
     * Find the neighbors of 'ndoeId'.
     */
    StatusWith<NodeList> getNeighbors(uint64_t nodeId);

    /**
     * Find the shortest path between 'nodeAId' and 'nodeBId'.
     */
    StatusWith<NodeList> shortestPath(uint64_t nodeAId, uint64_t nodeBId);
};

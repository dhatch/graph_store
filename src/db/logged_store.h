#pragma once

#include "db/log_manager.h"
#include "db/memory_store.h"
#include "db/types.h"
#include "util/nocopy.h"
#include "util/status.h"

/**
 * A graph store that maintains an on disk log and checkpoint in addition
 * to an in memory data representation.
 *
 * This enables durability.
 */
class LoggedStore : public GraphStore {
    DISALLOW_COPY(LoggedStore);
public:
    LoggedStore(const char* deviceName, bool formatLog);

    /**
     * Add a node with id `node_id` to the store.
     */
    Status addNode(NodeId nodeId) override;

    /**
     * Remove a node from the store.
     */
    Status removeNode(NodeId nodeId) override;

    /**
     * Find a node in the store.
     */
    StatusWith<Node*> findNode(NodeId nodeId) const override;

    /**
     * Find an edge in the store.
     */
    StatusWith<std::pair<Node*, Node*>> getEdge(NodeId nodeAId,
                                                NodeId nodeBId) const override;

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    Status addEdge(NodeId nodeAId, NodeId nodeBId) override;

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    Status removeEdge(NodeId nodeAId, NodeId nodeBId) override;

    /**
     * Find the neighbors of 'nodeId'.
     */
    StatusWith<NodeIdList> getNeighbors(NodeId nodeId) const override;

    /**
     * Find the length of the shortest path between 'nodeAId' and 'nodeBId'.
     *
     * Note, since there is no maximum distance, this implementation is
     * unbounded in its time and memory usage.
     */
    StatusWith<uint64_t> shortestPath(NodeId nodeAId,
                                      NodeId nodeBId) const override;

    /**
     * Recover operations from the on disk log.
     */
    void recover();
private:
    BufferManager _bufferManager;
    LogManager _log;
    MemoryStore _memoryStore;
};

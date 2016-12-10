#pragma once

#include <mutex>

#include "db/log_manager.h"
#include "db/checkpoint_manager.h"
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


    /**
     * Checkpoint into the checkpoint space.
     */
    Status checkpoint();

    /**
     * Recover operations from the on disk log.
     */
    void recover();

    friend class CheckpointManager;
private:
    BufferManager _bufferManager;
    LogManager _log;
    CheckpointManager _checkpoint;
    MemoryStore _memoryStore;

    mutable std::recursive_mutex _lock;
};

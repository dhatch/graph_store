/**
 * graph_store.h: Abstract interface for graph storage.
 */

#pragma once

#include <map>
#include <memory>
#include <utility>

#include "db/types.h"
#include "util/status.h"

class GraphStore {
    /**
     * Add a node with id `node_id` to the store.
     */
    virtual Status addNode(NodeId nodeId) = 0;

    /**
     * Remove a node from the store.
     */
    virtual Status removeNode(NodeId nodeId) = 0;

    /**
     * Find a node in the store.
     */
    virtual StatusWith<Node*> findNode(NodeId nodeId) const = 0;

    /**
     * Find an edge in the store.
     */
    virtual StatusWith<std::pair<Node*, Node*>> getEdge(NodeId nodeAId,
                                                NodeId nodeBId) const = 0;

    /**
     * Adds an edge between `nodeAId` and `nodeBId`.
     */
    virtual Status addEdge(NodeId nodeAId, NodeId nodeBId) = 0;

    /**
     * Remove an edge between 'nodeAId' and 'nodeBId'.
     */
    virtual Status removeEdge(NodeId nodeAId, NodeId nodeBId) = 0;

    /**
     * Find the neighbors of 'nodeId'.
     */
    virtual StatusWith<NodeIdList> getNeighbors(NodeId nodeId) const = 0;

    /**
     * Find the length of the shortest path between 'nodeAId' and 'nodeBId'.
     *
     * Note, since there is no maximum distance, this implementation is
     * unbounded in its time and memory usage.
     */
    virtual StatusWith<uint64_t> shortestPath(NodeId nodeAId,
                                              NodeId nodeBId) const = 0;
};

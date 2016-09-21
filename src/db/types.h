// Common data structures used throughout the database.

#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

using NodeId = uint64_t;

class Node {
public:
    Node(NodeId id) : _id(id) {};
    NodeId getId() const;
private:
    NodeId _id;

    /**
     * A set of edges.  Pointers are unowned, and view the data stored in
     * whatever store owns this node.
     *
     * The lifetime of the edges must extend that of this node.
     */
    std::unordered_set<Node*> _edges;

    friend class MemoryStore;
};

using NodeList = std::vector<Node>;

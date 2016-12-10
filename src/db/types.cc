#include "db/types.h"

#include <unordered_set>

NodeId Node::getId() const {
    return _id;
}

std::unordered_set<NodeId> Node::edges() const {
    return _edges;
}

bool Node::addEdge(NodeId node) {
    return _edges.insert(node).second;
}

bool Node::removeEdge(NodeId node) {
    auto it = _edges.find(node);
    if (it == _edges.end()) {
        return false;
    }

    _edges.erase(it);
    return true;
}

bool Node::hasEdge(NodeId node) const {
    return _edges.find(node) != _edges.end();
}

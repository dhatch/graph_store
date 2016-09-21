#include "db/types.h"

#include <unordered_set>

NodeId Node::getId() const {
    return _id;
}

std::unordered_set<Node*> Node::edges() const {
    return _edges;
}

bool Node::addEdge(Node* node) {
    return _edges.insert(node).second;
}

bool Node::removeEdge(Node* node) {
    auto it = _edges.find(node);
    if (it == _edges.end()) {
        return false;
    }

    _edges.erase(it);
    return true;
}

bool Node::hasEdge(Node* node) const {
    return _edges.find(node) != _edges.end();
}

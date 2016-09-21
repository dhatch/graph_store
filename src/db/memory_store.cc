#include "db/memory_store.h"

#include <utility>

#include "db/types.h"
#include "util/status.h"
#include "util/stdx/memory.h"

Status MemoryStore::addNode(NodeId nodeId) {
    auto inserted = _nodes.emplace(std::piecewise_construct, std::forward_as_tuple(nodeId),
                                   std::forward_as_tuple(stdx::make_unique<Node>(nodeId)));
    return inserted.second ? StatusCode::SUCCESS : StatusCode::NO_ACTION;
}

Status MemoryStore::removeNode(NodeId nodeId) {
    auto it = _nodes.find(nodeId);
    Node *node = nullptr;
    if (it == _nodes.end()) {
        return StatusCode::DOES_NOT_EXIST;
    }

    node = it->second.get();

    // Clean up edges
    for (auto&& neighbor : node->edges()) {
        neighbor->removeEdge(node);
    }

    _nodes.erase(it);
    return StatusCode::SUCCESS;
}

StatusWith<Node*> MemoryStore::findNode(NodeId nodeId) const {
    auto it = _nodes.find(nodeId);
    if (it == _nodes.end()) {
        return StatusCode::DOES_NOT_EXIST;
    }

    return it->second.get();
}

StatusWith<std::pair<Node*, Node*>> MemoryStore::getEdge(NodeId nodeAId, NodeId nodeBId) const {
    Node* nodeA = nullptr;
    Node* nodeB = nullptr;

    if (nodeAId == nodeBId) {
        return StatusCode::INVALID;
    }

    if (auto status = findNode(nodeAId)) {
        nodeA = *status;
    } else {
        return status.getCode();
    }

    if (auto status = findNode(nodeBId)) {
        nodeB = *status;
    } else {
        return status.getCode();
    }

    // Check that the edge exists.
    if (nodeA->hasEdge(nodeB)) {
        invariant(nodeB->hasEdge(nodeA));
        return {{nodeA, nodeB}};
    }

    invariant(!nodeB->hasEdge(nodeA));
    return StatusCode::DOES_NOT_EXIST;
}

Status MemoryStore::addEdge(NodeId nodeAId, NodeId nodeBId) {
    Node* nodeA = nullptr;
    Node* nodeB = nullptr;

    if (nodeAId == nodeBId) {
        return StatusCode::INVALID;
    }

    if (auto status = findNode(nodeAId)) {
        nodeA = *status;
    } else {
        return status.getCode();
    }

    if (auto status = findNode(nodeBId)) {
        nodeB = *status;
    } else {
        return status.getCode();
    }

    if (!nodeA->addEdge(nodeB)) {
        invariant(!nodeB->addEdge(nodeA));
        return StatusCode::NO_ACTION;
    }

    invariant(nodeB->addEdge(nodeA));

    return StatusCode::SUCCESS;
}

Status MemoryStore::removeEdge(NodeId nodeAId, NodeId nodeBId) {
    auto status = getEdge(nodeAId, nodeBId);
    if (!status) {
        return status;
    }

    Node* nodeA = status->first;
    Node* nodeB = status->second;

    if (!nodeA->removeEdge(nodeB)) {
        invariant(!nodeB->removeEdge(nodeA));
        return StatusCode::DOES_NOT_EXIST;
    }

    invariant(nodeB->removeEdge(nodeA));

    return StatusCode::SUCCESS;
}

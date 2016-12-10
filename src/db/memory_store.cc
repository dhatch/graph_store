#include "db/memory_store.h"

#include <deque>
#include <mutex>
#include <utility>

#include "db/types.h"
#include "util/status.h"
#include "util/stdx/memory.h"
#include "util/assert.h"

Status MemoryStore::addNode(NodeId nodeId) {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto inserted = _nodes.emplace(std::piecewise_construct, std::forward_as_tuple(nodeId),
                                   std::forward_as_tuple(stdx::make_unique<Node>(nodeId)));
    return inserted.second ? StatusCode::SUCCESS : StatusCode::NO_ACTION;
}

Status MemoryStore::removeNode(NodeId nodeId) {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto it = _nodes.find(nodeId);
    Node *node = nullptr;
    if (it == _nodes.end()) {
        return StatusCode::DOES_NOT_EXIST;
    }

    node = it->second.get();

    // Clean up edges
    for (const auto& neighborId : node->edges()) {
        auto neighbor = _nodes.find(neighborId);
        invariant(neighbor != _nodes.end());
        neighbor->second->removeEdge(nodeId);
    }

    _nodes.erase(it);
    return StatusCode::SUCCESS;
}

StatusWith<Node*> MemoryStore::findNode(NodeId nodeId) const {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto it = _nodes.find(nodeId);
    if (it == _nodes.end()) {
        return StatusCode::DOES_NOT_EXIST;
    }

    return it->second.get();
}

StatusWith<std::pair<Node*, Node*>> MemoryStore::getEdge(NodeId nodeAId, NodeId nodeBId) const {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

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
    if (nodeA->hasEdge(nodeBId)) {
        invariant(nodeB->hasEdge(nodeAId));
        return {{nodeA, nodeB}};
    }

    invariant(!nodeB->hasEdge(nodeAId));
    return StatusCode::DOES_NOT_EXIST;
}

Status MemoryStore::addEdge(NodeId nodeAId, NodeId nodeBId) {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

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

    if (!nodeA->addEdge(nodeBId)) {
        invariant(!nodeB->addEdge(nodeAId));
        return StatusCode::NO_ACTION;
    }

    invariant(nodeB->addEdge(nodeAId));

    return StatusCode::SUCCESS;
}

Status MemoryStore::removeEdge(NodeId nodeAId, NodeId nodeBId) {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto status = getEdge(nodeAId, nodeBId);
    if (!status) {
        return status;
    }

    Node* nodeA = status->first;
    Node* nodeB = status->second;

    if (!nodeA->removeEdge(nodeBId)) {
        invariant(!nodeB->removeEdge(nodeAId));
        return StatusCode::DOES_NOT_EXIST;
    }

    invariant(nodeB->removeEdge(nodeAId));

    return StatusCode::SUCCESS;
}

StatusWith<NodeIdList> MemoryStore::getNeighbors(NodeId nodeId) const {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto status_with_node = findNode(nodeId);
    if (!status_with_node) {
        return status_with_node.getCode();
    }

    const Node* node = *status_with_node;

    NodeIdList result;
    for (const NodeId& nId : node->edges()) {
        result.push_back(nId);
    }

    return result;
}

StatusWith<uint64_t> MemoryStore::shortestPath(NodeId nodeAId, NodeId nodeBId) const {
    std::lock_guard<std::recursive_mutex> lock(_memoryStoreMutex);

    auto status_with_node = findNode(nodeAId);
    if (!status_with_node) {
        return status_with_node.getCode();
    }

    if (!findNode(nodeBId)) {
        return StatusCode::DOES_NOT_EXIST;
    }

    if (nodeAId == nodeBId) {
       return StatusCode::NO_ACTION;
    }

    const Node* start = *status_with_node;

    // A depth first search.
    uint64_t distance = 0;
    std::deque<const Node*> toSearch = {start};
    std::deque<const Node*> nextSearch;
    std::unordered_set<NodeId> found;

    while (toSearch.size()) {
        for (const Node* n : toSearch) {
            // Check if we found our match.
            if (n->getId() == nodeBId) {
                return distance++;
            }

            // Add all the edges of n to nextSearch.
            for (const NodeId& edgeId : n->edges()) {
                if (!found.insert(edgeId).second) {
                    continue;
                }

                auto edgeIt = _nodes.find(edgeId);
                invariant(edgeIt != _nodes.end());
                const Node *edge = edgeIt->second.get();
                nextSearch.push_back(edge);
            }
        }

        distance++;
        toSearch = std::move(nextSearch);
        nextSearch.clear();
    }

    return StatusCode::NO_ACTION;
}

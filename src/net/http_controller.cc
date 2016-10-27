#include "net/http_controller.h"

#include "json/json.h"
#include "mongoose/JsonController.h"

#include "db/types.h"
#include "util/status.h"
#include "util/assert.h"

using namespace Mongoose;

namespace {
    void make400(JsonResponse& response) {
        response.setCode(400);
    }

    void make204(JsonResponse& response) {
        response.setCode(204);
    }

    void make507(JsonResponse& response) {
        response.setCode(507);
    }
}


StatusWith<Json::Value> HTTPController::getJSON(Request& request) {
    Json::Reader r;
    Json::Value v;
    if (!r.parse(request.getData(), v)) {
        return StatusCode::INVALID;
    }

    return v;
}

StatusWith<NodeId> HTTPController::getNodeId(Request &request, HatchResponse& response) {
    auto status_with_json = getJSON(request);

    if (!status_with_json) {
        make400(response);
        return StatusCode::INVALID;
    }

    Json::Value value = *status_with_json;
    Json::Value nodeId = value["node_id"];
    if (!nodeId.isUInt64()) {
        make400(response);
        return StatusCode::INVALID;
    }

    return nodeId.asUInt64();
}

StatusWith<std::pair<NodeId, NodeId>> HTTPController::getEdgeIds(Request &request, HatchResponse& response) {
    auto status_with_json = getJSON(request);

    if (!status_with_json) {
        make400(response);
        return StatusCode::INVALID;
    }

    Json::Value value = *status_with_json;
    Json::Value nodeAId = value["node_a_id"];
    if (!nodeAId.isUInt64()) {
        make400(response);
        return StatusCode::INVALID;
    }

    Json::Value nodeBId = value["node_b_id"];
    if (!nodeBId.isUInt64()) {
        make400(response);
        return StatusCode::INVALID;
    }

    return {{nodeAId.asUInt64(), nodeBId.asUInt64()}};
}

void HTTPController::add_node(Request& request, HatchResponse& response) {
    auto status_with_node_id  = getNodeId(request, response);
    if (!status_with_node_id) {
        return;
    }

    NodeId nodeId = *status_with_node_id;

    auto status = store.addNode(nodeId);
    if (status == StatusCode::NO_ACTION) {
        make204(response);
        return;
    } else if (!status) {
        make400(response);
        return;
    } else if (status == StatusCode::NO_SPACE) {
        make507(response);
        return;
    }

    response["node_id"] = nodeId;
    return;
}

void HTTPController::remove_node(Request& request, HatchResponse& response) {
    auto status_with_node_id = getNodeId(request, response);
    if (!status_with_node_id) {
        return;
    }

    NodeId nodeId = *status_with_node_id;

    auto status = store.removeNode(nodeId);
    if (status == StatusCode::NO_SPACE) {
        make507(response);
        return;
    } else if (!status) {
        make400(response);
        return;
    }

    response["node_id"] = nodeId;
    return;
}

void HTTPController::get_node(Request& request, HatchResponse& response) {
    auto status_with_node_id = getNodeId(request, response);
    if (!status_with_node_id) {
        return;
    }

    NodeId nodeId = *status_with_node_id;

    auto status = store.findNode(nodeId);

    response["in_graph"] = status.getCode() == StatusCode::SUCCESS;
    return;
}

void HTTPController::add_edge(Request& request, HatchResponse& response) {
    auto status_with_node_ids  = getEdgeIds(request, response);
    if (!status_with_node_ids) {
        return;
    }

    NodeId nodeAId = status_with_node_ids->first;
    NodeId nodeBId = status_with_node_ids->second;

    auto status = store.addEdge(nodeAId, nodeBId);
    if (status == StatusCode::NO_ACTION) {
        make204(response);
        return;
    } else if (!status) {
        make400(response);
        return;
    } else if (status == StatusCode::NO_SPACE) {
        make507(response);
        return;
    }

    response["node_a_id"] = nodeAId;
    response["node_b_id"] = nodeBId;
    return;
}

void HTTPController::remove_edge(Request& request, HatchResponse& response) {
    auto status_with_node_ids  = getEdgeIds(request, response);
    if (!status_with_node_ids) {
        return;
    }

    NodeId nodeAId = status_with_node_ids->first;
    NodeId nodeBId = status_with_node_ids->second;

    auto status = store.removeEdge(nodeAId, nodeBId);
    if (status == StatusCode::NO_SPACE) {
        make507(response);
        return;
    } else if (!status) {
        make400(response);
        return;
    }

    response["node_a_id"] = nodeAId;
    response["node_b_id"] = nodeBId;
    return;
}

void HTTPController::get_edge(Request& request, HatchResponse& response) {
    auto status_with_node_ids  = getEdgeIds(request, response);
    if (!status_with_node_ids) {
        return;
    }

    NodeId nodeAId = status_with_node_ids->first;
    NodeId nodeBId = status_with_node_ids->second;

    auto status = store.getEdge(nodeAId, nodeBId);

    response["in_graph"] = status.getCode() == StatusCode::SUCCESS;

    return;
}

void HTTPController::get_neighbors(Request& request, HatchResponse& response) {
    auto status_with_node_id = getNodeId(request, response);
    if (!status_with_node_id) {
        return;
    }

    NodeId nodeId = *status_with_node_id;

    auto status = store.getNeighbors(nodeId);
    if (!status) {
        make400(response);
        return;
    }

    Json::Value neighbors(Json::ValueType::arrayValue);
    for (auto&& node : *status) {
        neighbors.append(node);
    }

    response["node_id"] = nodeId;
    response["neighbors"] = neighbors;
    return;
}

void HTTPController::shortest_path(Request& request, HatchResponse& response) {
    auto status_with_node_ids  = getEdgeIds(request, response);
    if (!status_with_node_ids) {
        return;
    }

    NodeId nodeAId = status_with_node_ids->first;
    NodeId nodeBId = status_with_node_ids->second;

    auto status = store.shortestPath(nodeAId, nodeBId);
    if (status == StatusCode::NO_ACTION) {
        make204(response);
        return;
    } else if (!status) {
        make400(response);
        return;
    }

    response["distance"] = *status;

    return;
}

void HTTPController::setup() {
    setPrefix("/api/v1");
    addRouteResponse("POST", "/add_node", HTTPController, add_node, HatchResponse);
    addRouteResponse("POST", "/remove_node", HTTPController, remove_node, HatchResponse);
    addRouteResponse("POST", "/get_node", HTTPController, get_node, HatchResponse);
    addRouteResponse("POST", "/add_edge", HTTPController, add_edge, HatchResponse);
    addRouteResponse("POST", "/remove_edge", HTTPController, remove_edge, HatchResponse);
    addRouteResponse("POST", "/get_edge", HTTPController, get_edge, HatchResponse);
    addRouteResponse("POST", "/get_neighbors", HTTPController, get_neighbors, HatchResponse);
    addRouteResponse("POST", "/shortest_path", HTTPController, shortest_path, HatchResponse);
}

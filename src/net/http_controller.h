#include "mongoose/JsonController.h"

#include <utility>

#include "db/logged_store.h"
#include "net/hatch_response.h"

#pragma once

class HTTPController : public Mongoose::JsonController {
public:
    HTTPController(LoggedStore& store) : store(store) {};

    void add_node(Mongoose::Request& request, HatchResponse& response);
    void remove_node(Mongoose::Request& request, HatchResponse& response);
    void get_node(Mongoose::Request& request, HatchResponse& response);

    void add_edge(Mongoose::Request& request, HatchResponse& response);
    void remove_edge(Mongoose::Request& request, HatchResponse& response);
    void get_edge(Mongoose::Request& request, HatchResponse& response);

    void get_neighbors(Mongoose::Request& request, HatchResponse& response);
    void shortest_path(Mongoose::Request& request, HatchResponse& response);

    void setup();
private:
    StatusWith<Json::Value> getJSON(Mongoose::Request& request);
    StatusWith<NodeId> getNodeId(Mongoose::Request &request, HatchResponse& response);
    StatusWith<std::pair<NodeId, NodeId>> getEdgeIds(Mongoose::Request &request, HatchResponse& response);

    LoggedStore &store;
};

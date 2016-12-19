#include "mongoose/JsonController.h"

#include <utility>

#include "db/graph_store.h"
#include "db/replication_manager.h"
#include "db/partition/partition_config.h"
#include "db/partition/partition_manager.h"

#include "net/hatch_response.h"

#pragma once

class HTTPController : public Mongoose::JsonController {
public:
    HTTPController(GraphStore* store, ReplicationManager* replManager, PartitionConfig config, PartitionManager* manager, bool loggingEnabled) :
        store(store), replManager(replManager), partConfig(config), partManager(manager), loggingEnabled(loggingEnabled) {};

    void add_node(Mongoose::Request& request, HatchResponse& response);
    void remove_node(Mongoose::Request& request, HatchResponse& response);
    void get_node(Mongoose::Request& request, HatchResponse& response);

    void add_edge(Mongoose::Request& request, HatchResponse& response);
    void remove_edge(Mongoose::Request& request, HatchResponse& response);
    void get_edge(Mongoose::Request& request, HatchResponse& response);

    void get_neighbors(Mongoose::Request& request, HatchResponse& response);
    void shortest_path(Mongoose::Request& request, HatchResponse& response);

    void checkpoint(Mongoose::Request& request, HatchResponse& response);

    void setup();
private:
    StatusWith<Json::Value> getJSON(Mongoose::Request& request);
    StatusWith<NodeId> getNodeId(Mongoose::Request &request, HatchResponse& response);
    StatusWith<std::pair<NodeId, NodeId>> getEdgeIds(Mongoose::Request &request, HatchResponse& response);
    bool isPartitionedEdgeOp(NodeId nodeAId, NodeId nodeBId) const;

    void add_edge_partition(NodeId nodeAId, NodeId nodeBId, HatchResponse& response);
    void remove_edge_partition(NodeId nodeAId, NodeId nodeBId, HatchResponse &response);

    GraphStore *store;
    ReplicationManager *replManager;
    PartitionConfig partConfig;
    PartitionManager *partManager;
    bool loggingEnabled;
};

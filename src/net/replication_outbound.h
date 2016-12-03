#ifndef _NET_REPLICATION_OUTBOUND_H
#define _NET_REPLICATION_OUTBOUND_H

#include <memory>

#include <thrift/transport/TBufferTransports.h>

#include "util/nocopy.h"

#include "Replication.h"

using namespace apache::thrift::transport;

class ReplicationOutbound {
    DISALLOW_COPY(ReplicationOutbound);
public:
    ReplicationOutbound(const char* hostname, int port);
    ~ReplicationOutbound();

    bool addNode(NodeId nodeId);
    bool removeNode(NodeId nodeId);
    bool addEdge(NodeId nodeAId, NodeId nodeBId);
    bool removeEdge(NodeId nodeAId, NodeId nodeBId);
    ReplicationCheckpointResult::type checkpoint();

private:
    std::unique_ptr<ReplicationClient> _client;
    boost::shared_ptr<TTransport> _transport;
};

#endif

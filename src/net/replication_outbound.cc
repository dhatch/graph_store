#include "net/replication_outbound.h"

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "Replication.h"

#include "util/stdx/memory.h"

using namespace apache::thrift::protocol;

ReplicationOutbound::ReplicationOutbound(const char*hostname, int port) {
    boost::shared_ptr<TSocket> socket(new TSocket(hostname, port));
    _transport = boost::shared_ptr<TTransport>(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(_transport));

    _client = stdx::make_unique<ReplicationClient>(protocol);
    _transport->open();
}

ReplicationOutbound::~ReplicationOutbound() {
    _transport->close();
}

bool ReplicationOutbound::addNode(NodeId nodeId) {
    return _client->addNode(nodeId);
}


bool ReplicationOutbound::removeNode(NodeId nodeId) {
    return _client->removeNode(nodeId);
}

bool ReplicationOutbound::addEdge(NodeId nodeAId, NodeId nodeBId) {
    return _client->addEdge(nodeAId, nodeBId);
}

bool ReplicationOutbound::removeEdge(NodeId nodeAId, NodeId nodeBId) {
    return _client->removeEdge(nodeAId, nodeBId);
}

ReplicationCheckpointResult::type ReplicationOutbound::checkpoint() {
    return _client->checkpoint();
}

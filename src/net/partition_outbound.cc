#include "net/partition_outbound.h"

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "Partition.h"

#include "util/stdx/memory.h"

using namespace apache::thrift::protocol;

PartitionOutbound::PartitionOutbound(std::string hostname, int port) : _hostname(hostname) {
    boost::shared_ptr<TSocket> socket(new TSocket(_hostname, port));
    _transport = boost::shared_ptr<TTransport>(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(_transport));

    _client = stdx::make_unique<PartitionClient>(protocol);
    _transport->open();
}

PartitionOutbound::PartitionOutbound(PartitionOutbound&& other) : _hostname(other._hostname),
    _client(std::move(other._client)), _transport(other._transport) {
}

PartitionOutbound::~PartitionOutbound() {
    _transport->close();
}

bool PartitionOutbound::addEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) {
    return _client->addEdgePart(nodeLocalId, nodeRemoteId);
}


bool PartitionOutbound::removeEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) {
    return _client->removeEdgePart(nodeLocalId, nodeRemoteId);
}

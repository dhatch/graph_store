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
}

PartitionOutbound::PartitionOutbound(PartitionOutbound&& other) : _hostname(other._hostname),
    _client(std::move(other._client)), _transport(other._transport) {
}

PartitionOutbound::~PartitionOutbound() {
}

bool PartitionOutbound::addEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) {
    _transport->open();
    auto res = _client->addEdgePart(nodeLocalId, nodeRemoteId);
    _transport->close();
    return res;
}


bool PartitionOutbound::removeEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId) {
    _transport->open();
    auto res = _client->removeEdgePart(nodeLocalId, nodeRemoteId);
    _transport->close();
    return res;
}

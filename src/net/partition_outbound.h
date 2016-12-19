#ifndef _NET_PARTITION_OUTBOUND_H
#define _NET_PARTITION_OUTBOUND_H

#include <memory>

#include <thrift/transport/TBufferTransports.h>

#include "util/nocopy.h"

#include "Partition.h"

using namespace apache::thrift::transport;

class PartitionOutbound {
    DISALLOW_COPY(PartitionOutbound);
public:
    PartitionOutbound(std::string hostname, int port);
    PartitionOutbound(PartitionOutbound&& other);
    ~PartitionOutbound();

    bool addEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId);
    bool removeEdgePart(NodeId nodeLocalId, NodeId nodeRemoteId);

private:
    std::string _hostname;
    std::unique_ptr<PartitionClient> _client;
    boost::shared_ptr<TTransport> _transport;
};

#endif

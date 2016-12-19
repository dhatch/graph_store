#include "net/partition_server.h"

#include "Partition.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "util/assert.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class PartitionHandler : virtual public PartitionIf {
public:
    PartitionHandler(GraphStore* store) : _store(store) {};

    bool addEdgePart(const NodeId nodeLocalId, const NodeId nodeRemoteId) {
        return _store->addEdgePart(nodeLocalId, nodeRemoteId);
    }

    bool removeEdgePart(const NodeId nodeLocalId, const NodeId nodeRemoteId) {
        return _store->removeEdgePart(nodeLocalId, nodeRemoteId);
    }

private:
    GraphStore *_store;
};

PartitionServer::PartitionServer(std::string listenIp, int port, GraphStore *store)
    : _listenPort(port),
      _listenIp(listenIp),
      _store(store) {}

void PartitionServer::start() {
    shared_ptr<PartitionHandler> handler(new PartitionHandler(_store));
    shared_ptr<TProcessor> processor(new PartitionProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(_listenIp, _listenPort));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    server = stdx::make_unique<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);
    server->serve();
}

void PartitionServer::stop() {
    invariant(server);
    server->stop();
}

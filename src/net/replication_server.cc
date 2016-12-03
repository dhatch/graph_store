#include "net/replication_server.h"

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "db/logged_store.h"
#include "util/assert.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class ReplicationHandler : virtual public ReplicationIf {
public:
    ReplicationHandler(ReplicationManager* manager, GraphStore* store, bool loggingEnabled) : _replicationManager(manager), _store(store), _loggingEnabled(loggingEnabled) {};

    bool addNode(const NodeId nodeId) {
        if (!_store->addNode(nodeId)) {
            return false;
        }
        return _replicationManager->replicateAddNode(nodeId);
    }

    bool removeNode(const NodeId nodeId) {
        if (!_store->removeNode(nodeId)) {
            return false;
        }
        return _replicationManager->replicateRemoveNode(nodeId);
    }

    bool addEdge(const NodeId nodeAId, const NodeId nodeBId) {
        if (!_store->addEdge(nodeAId, nodeBId)) {
            return false;
        }
        return _replicationManager->replicateAddEdge(nodeAId, nodeBId);
    }

    bool removeEdge(const NodeId nodeAId, const NodeId nodeBId) {
        if (!_store->removeEdge(nodeAId, nodeBId)) {
            return false;
        }
        return _replicationManager->replicateRemoveEdge(nodeAId, nodeBId);
    }

    ReplicationCheckpointResult::type checkpoint() {
        if (!_loggingEnabled) {
            return ReplicationCheckpointResult::CHECKPOINT_DISABLED;
        }

        auto status = dynamic_cast<LoggedStore*>(_store)->checkpoint();
        if (!status) {
            return ReplicationCheckpointResult::OUT_OF_SPACE;
        }

        auto result = _replicationManager->replicateCheckpoint();
        switch (result) {
            case ReplicationManager::ReplicatedCheckpointStatus::CHECKPOINT_DISABLED:
                return ReplicationCheckpointResult::CHECKPOINT_DISABLED;
            case ReplicationManager::ReplicatedCheckpointStatus::OUT_OF_SPACE:
                return ReplicationCheckpointResult::OUT_OF_SPACE;
            case ReplicationManager::ReplicatedCheckpointStatus::SUCCESS:
                return ReplicationCheckpointResult::SUCCESS;
            default:
                return ReplicationCheckpointResult::CHECKPOINT_DISABLED;
        }

        return ReplicationCheckpointResult::SUCCESS;
    }
private:
    ReplicationManager *_replicationManager = nullptr;
    GraphStore *_store = nullptr;
    bool _loggingEnabled;
};

ReplicationServer::ReplicationServer(int port, GraphStore *store, ReplicationManager *manager,
        bool loggingEnabled) :
    _listenPort(port),
    _store(store),
    _replicationManager(manager),
    _loggingEnabled(loggingEnabled) {

}

void ReplicationServer::start() {
    std::cout << "ReplServer started." << std::endl;
    shared_ptr<ReplicationHandler> handler(new ReplicationHandler(_replicationManager, _store, _loggingEnabled));
    shared_ptr<TProcessor> processor(new ReplicationProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(_listenPort));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    server = stdx::make_unique<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);
    server->serve();
}

void ReplicationServer::stop() {
    invariant(server);
    server->stop();
}

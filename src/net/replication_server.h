#ifndef _NET_REPLICATION_SERVER_H_
#define _NET_REPLICATION_SERVER_H_

#include <thrift/server/TSimpleServer.h>

#include "Replication.h"
#include "db/graph_store.h"
#include "db/replication_manager.h"
#include "util/nocopy.h"
#include "util/stdx/memory.h"

using namespace ::apache::thrift::server;

class ReplicationServer {
    DISALLOW_COPY(ReplicationServer);
public:
    /**
     * Construct a 'ReplicationServer'.
     *
     * port: The port to listen for RPC calls on.
     * store: The graph store used for storage.
     * manager: The replication manager.
     * loggingEnabled: Whether *this* server has logging enabled.
     */
    ReplicationServer(int port, GraphStore *store, ReplicationManager *manager,
            bool loggingEnabled);

    void start();
    void stop();
private:
    int _listenPort;
    GraphStore *_store;
    ReplicationManager *_replicationManager;
    bool _loggingEnabled;

    std::unique_ptr<TSimpleServer> server = nullptr;
};

#endif /* _NET_REPLICATION_SERVER_H_ */

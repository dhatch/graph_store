#ifndef _NET_PARTITION_SERVER_H_
#define _NET_PARTITION_SERVER_H_

#include <thrift/server/TSimpleServer.h>

#include "Partition.h"
#include "db/graph_store.h"
#include "util/nocopy.h"
#include "util/stdx/memory.h"

using namespace ::apache::thrift::server;

class PartitionServer {
    DISALLOW_COPY(PartitionServer);
public:
    /**
     * Construct a 'PartitionServer'.
     *
     * port: The port to listen for RPC calls on.
     * store: The graph store used for storage.
     * manager: The replication manager.
     * loggingEnabled: Whether *this* server has logging enabled.
     */
    PartitionServer(std::string listenIp, int port, GraphStore *store);

    void start();
    void stop();
private:
    int _listenPort;
    std::string _listenIp;
    GraphStore *_store;

    std::unique_ptr<TSimpleServer> server = nullptr;
};

#endif /* _NET_REPLICATION_SERVER_H_ */

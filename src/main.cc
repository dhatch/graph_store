#include <cstdlib>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <thread>

#include "mongoose/Server.h"

#include "db/graph_store.h"
#include "db/logged_store.h"
#include "db/memory_store.h"
#include "db/replication_manager.h"

#include "db/partition/partition_config.h"
#include "db/partition/partition_manager.h"
#include "net/partition_server.h"

#include "net/http_controller.h"
#include "net/replication_server.h"
#include "net/replication_outbound.h"

#include "util/stdx/memory.h"

volatile static bool running = true;

void handle_signal(int sig)
{
    if (running) {
        cout << "Exiting..." << endl;
        running = false;
    }
}

static const char *USAGE =
    "cs426_graph_server: [-f] [-c] [-b ipaddress] portnum [devfile]\n"
    "Options:\n"
    "\t-f:\tFormat the <devfile> if provided on startup.\n"
    "\t-b ipaddress:\tThe ipaddress of the next successor in the replication chain.\n"
    "\t-c: This is a chain replica (not the head), and should not accept write commands over portnum.\n\n"
    "Arguments:\n"
    "portnum: The port to accept HTTP commands.\n"
    "devfile (optional): The device file to write to for durability.  If absent, durability is disabled.\n";

static const int RPC_PORT = 9091;

void die_with_usage() {
    std::cerr << USAGE << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    bool format = false;
    bool isChainReplica = false;
    char *replicationSuccessorIp = nullptr;

    int partNumber = -1;
    std::vector<std::string> addresses;

    int port = std::atoi(argv[optind++]);
    if (port == 0) {
        std::cerr << "Invalid port number." << std::endl;
        die_with_usage();
    }

    int opt;
    while ((opt = getopt(argc, argv, "fcb:p:l")) != -1) {
        switch (opt) {
            case 'f':
                format = true;
                break;
            case 'c':
                isChainReplica = true;
                break;
            case 'b':
                replicationSuccessorIp = optarg;
                break;
            case 'p':
                partNumber = std::atoi(optarg) - 1;
                if (partNumber == -1) {
                    std::cerr << "Invalid part number." << std::endl;
                    die_with_usage();
                }
                break;
            case 'l':
                break;
            case '?':
            default:
                die_with_usage();
        }
    }

    while (optind < argc) {
        addresses.emplace_back(argv[optind++]);
    }

    for (auto addr : addresses) {
        std::cerr << addr << std::endl;
    }

    srand(time(NULL));
    signal(SIGINT, handle_signal);

    std::unique_ptr<GraphStore> store;
    store = stdx::make_unique<MemoryStore>();

    std::unique_ptr<ReplicationManager> replManager = nullptr;
    ReplicationManager::NodeType type;
    std::unique_ptr<ReplicationOutbound> replicationOutbound = nullptr;
    if (replicationSuccessorIp || isChainReplica) {
        if (replicationSuccessorIp) {
            if (isChainReplica) {
                type = ReplicationManager::NodeType::MID;
            } else {
                type = ReplicationManager::NodeType::HEAD;
            }
            replicationOutbound = stdx::make_unique<ReplicationOutbound>(
                    replicationSuccessorIp, RPC_PORT);
        } else {
            type = ReplicationManager::NodeType::TAIL;
        }
        replManager = stdx::make_unique<ReplicationManager>(
                type, store.get(), replicationOutbound.get());
    }

    PartitionConfig config(addresses, partNumber);
    PartitionManager partitionManager(config);

    Mongoose::Server server(port);
    HTTPController controller(store.get(), replManager.get(), config, &partitionManager, false);
    server.registerController(&controller);
    server.setOption("enable_directory_listing", "false");

    // TODO there is probably something wrong here.
    server.start();

    std::unique_ptr<ReplicationServer> replServer = nullptr;
    std::thread replServerThread;
    if (replManager) {
        replServer = stdx::make_unique<ReplicationServer>(
                RPC_PORT, store.get(), replManager.get(),
                false);

        if (type != ReplicationManager::NodeType::HEAD) {
            replServerThread = std::thread([&]{
                replServer->start();
            });
        }
    }

    std::unique_ptr<PartitionServer> partServer = nullptr;
    std::thread partitionServerThread;
    if (partNumber != -1) {
        const std::string& ip = config.partitions()[config.us()];
        auto pos = ip.find_last_of(':');
        if (pos == std::string::npos) {
            die_with_usage();
        }

        partServer = stdx::make_unique<PartitionServer>(ip.substr(0, pos),
                std::atoi(ip.substr(pos + 1).c_str()), store.get());
        partitionServerThread = std::thread([&]{
            partServer->start();
        });
    }

    cout << "Server started on port: " << port << ", routes:" << endl;
    controller.dumpRoutes();

    while (running) {
        sleep(100);
    }

    server.stop();
    if (replServer) {
        replServer->stop();
        replServerThread.join();
    }

    if (partServer) {
        partServer->stop();
        partitionServerThread.join();
    }

    return EXIT_SUCCESS;
}

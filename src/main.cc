#include <cstdlib>
#include <csignal>
#include <iostream>
#include <unistd.h>

#include "mongoose/Server.h"

#include "db/graph_store.h"
#include "db/logged_store.h"
#include "db/memory_store.h"
#include "net/http_controller.h"

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

void die_with_usage() {
    std::cerr << USAGE << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    bool format = false;
    bool isChainReplica = false;
    char *replicationSuccessorIp = nullptr;

    int opt;
    while ((opt = getopt(argc, argv, "fcb:")) != -1) {
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
            case '?':
            default:
                die_with_usage();
        }
    }

    int port = 0;
    const char *devPath = nullptr;

    if (optind >= argc) {
        std::cerr << "Missing port number." << std::endl;
        die_with_usage();
    }

    port = std::atoi(argv[optind++]);
    if (port == 0) {
        std::cerr << "Invalid port number." << std::endl;
        die_with_usage();
    }

    if (optind < argc) {
        devPath = argv[optind];
    }

    srand(time(NULL));
    signal(SIGINT, handle_signal);

    GraphStore *store = nullptr;
    if (devPath) {
        store = new LoggedStore(devPath, format);
    } else {
        store = new MemoryStore();
    }

    Mongoose::Server server(port);
    HTTPController controller(store, devPath != nullptr);
    server.registerController(&controller);
    server.setOption("enable_directory_listing", "false");
    server.start();

    cout << "Server started on port: " << port << ", routes:" << endl;
    controller.dumpRoutes();

    while (running) {
        sleep(100);
    }

    server.stop();

    return EXIT_SUCCESS;
}

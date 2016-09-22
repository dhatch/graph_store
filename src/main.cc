#include "net/http_controller.h"

#include <cstdlib>

#include "mongoose/Server.h"

volatile static bool running = true;

void handle_signal(int sig)
{
    if (running) {
        cout << "Exiting..." << endl;
        running = false;
    }
}

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        return EXIT_FAILURE;
    }

    char* port_c = argv[1];
    int port = std::atoi(port_c);

    srand(time(NULL));

    signal(SIGINT, handle_signal);

    HTTPController controller;
    Mongoose::Server server(port);
    server.registerController(&controller);
    server.setOption("enable_directory_listing", "false");
    server.start();

    cout << "Server started, routes:" << endl;
    controller.dumpRoutes();

    while (running) {
        sleep(10);
    }

    server.stop();

    return EXIT_SUCCESS;
}

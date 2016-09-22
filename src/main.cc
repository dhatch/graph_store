#include "net/http_controller.h"

#include "mongoose/Server.h"

volatile static bool running = true;

void handle_signal(int sig)
{
    if (running) {
        cout << "Exiting..." << endl;
        running = false;
    }
}

int main()
{
    srand(time(NULL));

    signal(SIGINT, handle_signal);

    HTTPController controller;
    Mongoose::Server server(8080);
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

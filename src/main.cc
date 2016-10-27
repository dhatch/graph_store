#include "net/http_controller.h"


#include <cstdlib>
#include <csignal>
#include <unistd.h>

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
    if (argc <= 2) {
        return EXIT_FAILURE;
    }

    char** arg = argv + 1;
    bool format = false;
    if (strcmp(*arg, "-f") == 0) {
        format = true;
        arg++;
    }

    char* port_c = *(arg++);
    int port = std::atoi(port_c);

    char* devPath = *(arg++);

    srand(time(NULL));

    signal(SIGINT, handle_signal);

    LoggedStore loggedStore(devPath, format);
    HTTPController controller(loggedStore);
    Mongoose::Server server(port);
    server.registerController(&controller);
    server.setOption("enable_directory_listing", "false");
    server.start();

    cout << "Server started on port: " << port << ", routes:" << endl;
    controller.dumpRoutes();

    while (running) {
        sleep(10);
    }

    server.stop();

    return EXIT_SUCCESS;
}

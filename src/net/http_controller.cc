#include "net/http_controller.h"

#include "mongoose/JsonController.h"

using namespace Mongoose;

void HTTPController::hello(Request& request, JsonResponse& response) {
    response["hello"] = "world";
}

void HTTPController::setup() {
    addRouteResponse("GET", "/", HTTPController, hello, JsonResponse);
}

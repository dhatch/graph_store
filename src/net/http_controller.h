#include "mongoose/JsonController.h"

#pragma once

class HTTPController : public Mongoose::JsonController {
public:
    void hello(Mongoose::Request& request, Mongoose::JsonResponse& response);
    void setup();
};

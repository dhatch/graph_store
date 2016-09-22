#include "mongoose/JsonResponse.h"

#include <string>

#pragma once

class HatchResponse : public Mongoose::JsonResponse {
public:
    HatchResponse() = default;

    virtual std::string getBody();
};

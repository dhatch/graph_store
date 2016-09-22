#include "net/hatch_response.h"

#include <string>

std::string HatchResponse::getBody() {
    if (code == 204) {
        return {};
    }

    std::string body = JsonResponse::getBody();
    return body;
}


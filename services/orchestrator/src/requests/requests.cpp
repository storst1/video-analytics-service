#include "requests.h"

#include <exception>

namespace requests {

std::string VideoStatusToString(const VideoStatus& status) {
    switch (status) {
    case VideoStatus::Received:
        return "Received";
    case VideoStatus::PreProcessing:
        return "PreProcessing";
    default:
        throw std::exception("Unknown VideoStatus at VideoStatusToString()");
    }
}

} // namespace requests
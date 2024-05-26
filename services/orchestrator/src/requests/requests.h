#pragma once

#include <string>

namespace requests {

enum class VideoStatus {
    Received = 1,
    PreProcessing = 2
};

struct VideoRequest {
    std::string id;
    std::string path;
    VideoStatus status;
};

std::string VideoStatusToString(const VideoStatus& status);

} // namespace requests

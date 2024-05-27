#pragma once

#include <string>

namespace requests {

enum class VideoStatus {
    Received = 1,
    PreProcessingFrames = 2,
    PreProcessingResize = 3,
    YoloStarted = 4,
    YoloFinished = 5,
    FramesCleanUp = 6,
    PostProcessing = 7,
    Finished = 8,
    Failed = 9
};

struct VideoRequest {
    std::string id;
    std::string path;
    VideoStatus status;
};

std::string VideoStatusToString(const VideoStatus& status);

} // namespace requests

#include "requests.h"

#include <exception>

namespace requests {

std::string VideoStatusToString(const VideoStatus& status) {
    switch (status) {
    case VideoStatus::Received:
        return "Received";
    case VideoStatus::PreProcessingFrames:
        return "PreProcessingFrames";
    case VideoStatus::PreProcessingResize:
        return "PreProcessingResize";
    case VideoStatus::YoloStarted:
        return "YoloStarted";
    case VideoStatus::YoloFinished:
        return "YoloFinished";
    case VideoStatus::FramesCleanUp:
        return "FramesCleanUp";
    case VideoStatus::PostProcessing:
        return "PostProcessing";
    case VideoStatus::Finished:
        return "Finished";
    case VideoStatus::Failed:
        return "Failed";
    default:
        throw std::exception("Unknown VideoStatus at VideoStatusToString()");
    }
}

} // namespace requests
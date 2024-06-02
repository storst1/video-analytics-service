#include "requests.h"

#include <exception>

namespace requests {

/**
 * Converts a VideoStatus enum value to its corresponding string representation.
 * 
 * @param status The VideoStatus enum value to convert.
 * @return The string representation of the VideoStatus enum value.
 * @throws std::exception if the VideoStatus enum value is unknown.
 */
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
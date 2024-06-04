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
    case VideoStatus::PreProcessingStarted:
        return "PreProcessingStarted";
    case VideoStatus::PreProcessingFinished:
        return "PreProcessingFinished";
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
    case VideoStatus::Stopped:
        return "Stopped";
    default:
        throw std::exception("Unknown VideoStatus at VideoStatusToString()");
    }
}

/**
 * Converts a string representation of VideoStatus to its corresponding enum value.
 * 
 * @param statusStr The string representation of VideoStatus to convert.
 * @return The VideoStatus enum value.
 * @throws std::exception if the string representation is unknown.
 */
VideoStatus StringToVideoStatus(const std::string& statusStr) {
    if (statusStr == "Received") {
        return VideoStatus::Received;
    } else if (statusStr == "PreProcessingStarted") {
        return VideoStatus::PreProcessingStarted;
    } else if (statusStr == "PreProcessingFinished") {
        return VideoStatus::PreProcessingFinished;
    } else if (statusStr == "YoloStarted") {
        return VideoStatus::YoloStarted;
    } else if (statusStr == "YoloFinished") {
        return VideoStatus::YoloFinished;
    } else if (statusStr == "FramesCleanUp") {
        return VideoStatus::FramesCleanUp;
    } else if (statusStr == "PostProcessing") {
        return VideoStatus::PostProcessing;
    } else if (statusStr == "Finished") {
        return VideoStatus::Finished;
    } else if (statusStr == "Failed") {
        return VideoStatus::Failed;
    } else if (statusStr == "Stopped") {
        return VideoStatus::Stopped;
    } else {
        throw std::exception("Unknown string representation at StringToVideoStatus()");
    }
}

} // namespace requests
cmake_minimum_required(VERSION 3.10)
project(video_pre_processing)

# Set C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the path to Asio relative to the project's root directory
set(ASIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../deps/asio/asio/include")

# Print a message with the path to Asio
message(STATUS "ASIO_INCLUDE_DIR is set to ${ASIO_INCLUDE_DIR}")

include(FetchContent)

# Download and build Crow
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG master
)

FetchContent_MakeAvailable(crow)

# Set the path to the header files
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Define the path to the FFmpeg executables
if(WIN32)
    set(FFMPEG_EXECUTABLE "${CMAKE_SOURCE_DIR}/../../deps/ffmpeg/windows/bin/ffmpeg.exe")
elseif(UNIX)
    set(FFMPEG_EXECUTABLE "${CMAKE_SOURCE_DIR}/../../deps/ffmpeg/linux/bin/ffmpeg")
endif()

# Print a message with the path to FFmpeg
message(STATUS "FFMPEG_EXECUTABLE is set to ${FFMPEG_EXECUTABLE}")

# Pass the FFmpeg path to the code through a macro
add_definitions(-DFFMPEG_EXECUTABLE=\"${FFMPEG_EXECUTABLE}\")

# Set the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries (if additional libraries are needed, add them here)
target_link_libraries(${PROJECT_NAME} ${crow_LIBRARIES})

# Set flags for NASM (if NASM is used)
if(NOT MSVC)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -g cv8 -D__x86_64__")
endif()

cmake_minimum_required(VERSION 3.14)
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

# Загрузка и сборка hiredis
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG master
)

FetchContent_MakeAvailable(hiredis)

# Set the path to the header files
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../../utils/redis)
include_directories(${CMAKE_SOURCE_DIR}/../../utils/http)
include_directories(${CMAKE_SOURCE_DIR}/../../utils/cfg)

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
file(GLOB_RECURSE SOURCES "src/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/redis/*.cpp" 
                          "${CMAKE_SOURCE_DIR}/../../utils/http/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/cfg/*.cpp" )
file(GLOB_RECURSE HEADERS "include/*.h" "${CMAKE_SOURCE_DIR}/../../utils/redis/*.h" 
                          "${CMAKE_SOURCE_DIR}/../../utils/http/*.h" "${CMAKE_SOURCE_DIR}/../../utils/cfg/*.h" )

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries (if additional libraries are needed, add them here)
target_link_libraries(video_pre_processing PRIVATE ${crow_LIBRARIES} hiredis)

# Win32-specific definitions and link libraries
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    target_link_libraries(video_pre_processing PRIVATE ws2_32)
endif()

if (WIN32)
    add_custom_command(TARGET video_pre_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:video_pre_processing>)
endif()

cmake_minimum_required(VERSION 3.14)
project(frame_analysis)

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

# Download and build hiredis
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG master
)

FetchContent_MakeAvailable(hiredis)

# Set include paths
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${hiredis_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../utils/redis)
include_directories(${CMAKE_SOURCE_DIR}/../utils/http)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/redis/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/http/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "${CMAKE_SOURCE_DIR}/../../utils/redis/*.h" "${CMAKE_SOURCE_DIR}/../../utils/http/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries (add additional libraries here if needed)
target_link_libraries(${PROJECT_NAME} ${crow_LIBRARIES} hiredis)

# Copy hiredisd.dll to the directory with the executable file
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# Set flags for NASM (if NASM is used)
if(NOT MSVC)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -g cv8 -D__x86_64__")
endif()

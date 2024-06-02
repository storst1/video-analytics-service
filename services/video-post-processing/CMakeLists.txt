cmake_minimum_required(VERSION 3.14)

project(video_post_processing)

# Установка политик
cmake_policy(SET CMP0002 NEW)
cmake_policy(SET CMP0079 NEW)

# Установка стандартов C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Включение генерации compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Определение пути к Asio относительно корневой директории проекта
set(ASIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../deps/asio/asio/include")

# Вывод сообщения с путем к Asio
message(STATUS "ASIO_INCLUDE_DIR is set to ${ASIO_INCLUDE_DIR}")

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lws2_32")
endif()

set(POSTGRESQL_USER_PATH "D:/Program Files/PostgreSQL/15")

message(STATUS "POSTGRESQL_USER_PATH is set to ${POSTGRESQL_USER_PATH}")

set(PostgreSQL_ADDITIONAL_VERSIONS "15.3")
set(PostgreSQL_ROOT "${POSTGRESQL_USER_PATH}")
set(PostgreSQL_LIBRARY_DIR "${POSTGRESQL_USER_PATH}/lib")
set(PostgreSQL_INCLUDE_DIR "${POSTGRESQL_USER_PATH}/include")

# Поиск локально установленного PostgreSQL
find_package(PostgreSQL REQUIRED)

include(FetchContent)

# Загрузка и сборка Crow
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

# Установка пути к заголовочным файлам
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../../utils/redis)
include_directories(${CMAKE_SOURCE_DIR}/../../utils/http)
include_directories(${CMAKE_SOURCE_DIR}/../../utils/db)
include_directories(${hiredis_SOURCE_DIR})
include_directories(${PostgreSQL_INCLUDE_DIR})

# Добавление исходных файлов
file(GLOB_RECURSE SOURCES "src/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "${CMAKE_SOURCE_DIR}/../../utils/*.h")

add_executable(video_post_processing ${SOURCES})

add_subdirectory(${CMAKE_SOURCE_DIR}/../../deps/libpqxx build-pqxx)

target_link_libraries(video_post_processing PRIVATE ${crow_LIBRARIES} hiredis pqxx PostgreSQL::PostgreSQL)

# Win32-specific definitions and link libraries
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    target_link_libraries(video_post_processing PRIVATE ws2_32)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/build/build-pqxx/src/pqxx.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${POSTGRESQL_USER_PATH}/bin/libpq.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${POSTGRESQL_USER_PATH}/bin/libssl-3-x64.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${POSTGRESQL_USER_PATH}/bin/libcrypto-3-x64.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${POSTGRESQL_USER_PATH}/bin/libintl-9.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()
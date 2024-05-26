cmake_minimum_required(VERSION 3.10)

project(orchestrator)

# Установка стандартов C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Включение генерации compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Определение пути к Asio относительно корневой директории проекта
set(ASIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../deps/asio/asio/include")

# Вывод сообщения с путем к Asio
message(STATUS "ASIO_INCLUDE_DIR is set to ${ASIO_INCLUDE_DIR}")

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

message(STATUS "hiredis_SOURCE_DIR is set to ${hiredis_SOURCE_DIR}")
message(STATUS "hiredis_BINARY_DIR is set to ${hiredis_BINARY_DIR}")

# Установка путей к заголовочным файлам
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${hiredis_SOURCE_DIR})
include_directories(${hiredis_BINARY_DIR})

# Добавление исходных файлов
file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка библиотек
target_link_libraries(${PROJECT_NAME} hiredis)

# Копирование hiredis.dll в директорию с исполняемым файлом
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

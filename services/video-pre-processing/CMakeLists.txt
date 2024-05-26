cmake_minimum_required(VERSION 3.10)

project(video_pre_processing)

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

# Установка путей к заголовочным файлам Crow и Asio
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})

# Добавление исходных файлов
file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Так как Crow это заголовочная библиотека, нет необходимости явно указывать линковку

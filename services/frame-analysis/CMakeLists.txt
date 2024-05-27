cmake_minimum_required(VERSION 3.10)
project(frame_analysis)

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

# Установка пути к заголовочным файлам
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Установка исходных файлов
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка библиотек (если есть необходимость в дополнительных библиотеках, добавьте их здесь)
target_link_libraries(${PROJECT_NAME} ${crow_LIBRARIES})

# Установка флагов для NASM (если используется NASM)
if(NOT MSVC)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -g cv8 -D__x86_64__")
endif()

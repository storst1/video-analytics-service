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

set(PostgreSQL_ADDITIONAL_VERSIONS "15.3")
# set(PostgreSQL_ROOT "D:/Program Files/PostgreSQL/15")
# set(PostgreSQL_LIBRARY_DIR "D:/Program Files/PostgreSQL/15/lib")
# set(PostgreSQL_INCLUDE_DIR "D:/Program Files/PostgreSQL/15/include")

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

# Загрузка и сборка libpqxx
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.7.4
)

FetchContent_MakeAvailable(libpqxx)

# Установка пути к заголовочным файлам
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../utils/redis)
include_directories(${CMAKE_SOURCE_DIR}/../utils/http)
include_directories(${CMAKE_SOURCE_DIR}/../utils/db)
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${crow_SOURCE_DIR}/include)

# Добавление исходных файлов
file(GLOB_RECURSE SOURCES "src/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "${CMAKE_SOURCE_DIR}/../../utils/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка библиотек
target_link_libraries(${PROJECT_NAME} PRIVATE ${crow_LIBRARIES} hiredis pqxx PostgreSQL::PostgreSQL)

# Копирование hiredisd.dll в директорию с исполняемым файлом (если необходимо)
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# Определение _WIN32_WINNT
if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0A00)
else()
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

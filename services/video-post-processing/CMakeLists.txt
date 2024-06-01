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

set(PostgreSQL_ADDITIONAL_VERSIONS "15.3")
set(PostgreSQL_ROOT "D:/Program Files/PostgreSQL/15")
set(PostgreSQL_LIBRARY_DIR "D:/Program Files/PostgreSQL/15/lib")
set(PostgreSQL_INCLUDE_DIR "D:/Program Files/PostgreSQL/15/include")

# Поиск локально установленного PostgreSQL
find_package(PostgreSQL REQUIRED)

include(FetchContent)
include(ExternalProject)

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

# Загрузка и сборка OpenSSL
ExternalProject_Add(
    openssl
    PREFIX ${CMAKE_BINARY_DIR}/openssl
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG OpenSSL_1_1_1-stable
    CONFIGURE_COMMAND perl Configure VC-WIN64A --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>
    BUILD_COMMAND cmake --build . --config Release
    INSTALL_COMMAND cmake --install . --config Release
    BUILD_IN_SOURCE 1
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
)

# Переменные для путей OpenSSL
set(OPENSSL_INCLUDE_DIR ${CMAKE_BINARY_DIR}/openssl/include)
set(OPENSSL_CRYPTO_LIBRARY ${CMAKE_BINARY_DIR}/openssl/lib/libcrypto.a)

# Создание фиктивной (заглушки) цели для OpenSSL
add_custom_target(openssl_ready
    DEPENDS openssl
)

# Импортируемую библиотеку crypto зависим от целевого объекта openssl_ready
add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES
    IMPORTED_LOCATION ${OPENSSL_CRYPTO_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR}
)

# add_dependencies(video_post_processing openssl_ready)

# Установка пути к заголовочным файлам
include_directories(${crow_SOURCE_DIR}/include)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${LIBPQXX_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../utils/redis)
include_directories(${CMAKE_SOURCE_DIR}/../utils/http)
include_directories(${CMAKE_SOURCE_DIR}/../utils/db)
include_directories(${hiredis_SOURCE_DIR})
include_directories(${PostgreSQL_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})

# Добавление исходных файлов
file(GLOB_RECURSE SOURCES "src/*.cpp" "${CMAKE_SOURCE_DIR}/../../utils/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "${CMAKE_SOURCE_DIR}/../../utils/*.h")

add_executable(video_post_processing ${SOURCES})

target_link_libraries(video_post_processing PRIVATE ${crow_LIBRARIES} openssl_ready hiredis pqxx PostgreSQL::PostgreSQL)

# Win32-specific definitions and link libraries
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    target_link_libraries(video_post_processing PRIVATE ws2_32)
endif()

# Копирование hiredisd.dll в директорию с исполняемым файлом (если необходимо)
if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${hiredis_BINARY_DIR}/hiredisd.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()

# Копирование pqxx.dll в директорию с исполняемым файлом (если необходимо)
if (WIN32)
    add_custom_command(TARGET video_post_processing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/build/_deps/libpqxx-build/src/pqxx.dll"
        $<TARGET_FILE_DIR:video_post_processing>)
endif()
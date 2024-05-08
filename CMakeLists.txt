cmake_minimum_required(VERSION 3.12)
project(VideoAnalysisSystem)

# Установка стандарта C++ и необходимых компиляторов
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

# Интеграция с vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")

# Путь к PowerShell Core, для использования в кастомных командах CMake
set(PWSH_PATH "D:/Soft/PowerShell/pwsh.exe")

# Настройки проекта
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/services/video-pre-processing/include
)

# Список исходных файлов
set(SOURCE_FILES
    services/video-pre-processing/src/main.cpp
)

# Создание исполняемого файла
add_executable(VideoAnalysisSystem ${SOURCE_FILES})

# Находим и подключаем OpenCV
find_package(OpenCV REQUIRED)
target_include_directories(VideoAnalysisSystem PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(VideoAnalysisSystem PRIVATE ${OpenCV_LIBS})

# Настройки для кастомных команд с использованием PowerShell
add_custom_command(
    TARGET VideoAnalysisSystem POST_BUILD
    COMMAND ${PWSH_PATH} -ExecutionPolicy Bypass -noprofile -Command "Write-Host 'Post Build Script Running...'"
    COMMENT "Running PowerShell post-build script"
)

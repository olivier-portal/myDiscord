cmake_minimum_required(VERSION 3.10)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable verbose makefile output
set(CMAKE_VERBOSE_MAKEFILE ON)

# Récupère les fichiers source
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
message(STATUS "Sources détectées : ${SOURCES}")

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

# Manually specify PostgreSQL paths
include_directories(
    ${GTK_INCLUDE_DIRS}
    "C:/msys64/ucrt64/include" 
    "C:/Program Files/PostgreSQL/17/include"
)

link_directories(
    ${GTK_LIBRARY_DIRS}
    "C:/msys64/ucrt64/lib" 
    "C:/Program Files/PostgreSQL/17/lib"
)

target_link_libraries(${PROJECT_NAME}
    ${GTK_LIBRARIES}
    "C:/Program Files/PostgreSQL/17/lib/libpq.lib"
)

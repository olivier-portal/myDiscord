cmake_minimum_required(VERSION 3.10)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Récupère les fichiers source
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
message(STATUS "Sources détectées : ${SOURCES}")

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
pkg_check_modules(PostgreSQL REQUIRED libpq)

include_directories(
    ${GTK_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${GTK_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
)

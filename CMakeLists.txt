cmake_minimum_required(VERSION 3.20)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

# Trouve GTK et PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

# Manually specify PostgreSQL paths
include_directories(
    ${GTK_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
)

# Fichier principal
file(GLOB_RECURSE SOURCES "src/*.c")

message(STATUS "Sources détectées : ${SOURCES}")

add_executable(server
  src/server/server_main.c
  src/server/server_socket.c
  src/config/config.c
)
target_link_libraries(server ws2_32 ${PostgreSQL_LIBRARIES})

add_executable(client
  src/client/client_main.c
  src/client/client_socket.c
  src/config/config.c
  src/ui/window.c
)
target_link_libraries(client ${GTK_LIBRARIES} ${PostgreSQL_LIBRARIES} ws2_32)



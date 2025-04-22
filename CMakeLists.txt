cmake_minimum_required(VERSION 3.20)
project(myDiscord C)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
pkg_check_modules(PostgreSQL REQUIRED libpq)

include_directories(
    ${GTK_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
)

# Récupère les fichiers source
file(GLOB_RECURSE SOURCES "src/*.c")
message(STATUS "Sources détectées : ${SOURCES}")

add_executable(myDiscord ${SOURCES})
target_link_libraries(myDiscord
    ${GTK_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
)

add_executable(client
    src/client/client_socket.c
    src/config/config.c
    src/ui/window.c
)
target_include_directories(client PRIVATE src)
target_link_libraries(client
    ${GTK_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(server
  src/server/server_main.c
  src/server/server_socket.c
  src/config/config.c
)

target_include_directories(server PRIVATE src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

target_link_libraries(server PRIVATE
  ${LIBPQ_LIBRARIES}
  ws2_32
  kernel32 user32 gdi32 winspool shell32
  ole32 oleaut32 uuid comdlg32 advapi32
)

target_compile_definitions(server PRIVATE ${LIBPQ_CFLAGS_OTHER})
target_include_directories(server PRIVATE ${LIBPQ_INCLUDE_DIRS})


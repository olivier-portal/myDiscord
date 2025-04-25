cmake_minimum_required(VERSION 3.20)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

# Find GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

# Manually specify PostgreSQL paths
set(PostgreSQL_LIBRARY_DIR "C:/Program Files/PostgreSQL/17/lib")
set(PostgreSQL_INCLUDE_DIR "C:/Program Files/PostgreSQL/17/include")

if (EXISTS ${PostgreSQL_LIBRARY_DIR} AND EXISTS ${PostgreSQL_INCLUDE_DIR})
    message(STATUS "PostgreSQL paths set manually.")
    include_directories(${PostgreSQL_INCLUDE_DIR})
    link_directories(${PostgreSQL_LIBRARY_DIR})
else()
    message(FATAL_ERROR "PostgreSQL paths are invalid. Please check the paths in CMakeLists.txt.")
endif()

# Specify PostgreSQL library files explicitly
set(PostgreSQL_LIBRARIES
    "${PostgreSQL_LIBRARY_DIR}/libpq.lib" # Adjust this to match the actual library file name
)

# Include directories
include_directories(${GTK_INCLUDE_DIRS})

# Link directories
link_directories(${GTK_LIBRARY_DIRS})

# Define source files
file(GLOB_RECURSE SOURCES "src/*.c")

message(STATUS "Sources détectées : ${SOURCES}")

# Server executable
add_executable(server
    src/server/server_main.c
    src/server/server_socket.c
    src/config/config.c
    src/database/channel_crud.c
    src/database/client_crud.c
    src/database/db_connection.c
    src/database/message_crud.c
    src/database/reaction_crud.c
)
target_link_libraries(server ws2_32 ${PostgreSQL_LIBRARIES})

# Client executable
add_executable(client
    src/client/client_main.c
    src/client/client_socket.c
    src/config/config.c
    src/ui/window.c
)
target_link_libraries(client ${GTK_LIBRARIES} ${PostgreSQL_LIBRARIES} ws2_32)



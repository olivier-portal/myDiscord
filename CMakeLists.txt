cmake_minimum_required(VERSION 3.20)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

# Trouve GTK et PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
pkg_check_modules(PostgreSQL REQUIRED libpq)

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

# Chemin vers le fichier CSS source
set(CSS_SOURCE "${CMAKE_SOURCE_DIR}/src/ui/custom.css")

# Dossier de destination après le build (build dir racine)
set(CSS_DESTINATION "${CMAKE_BINARY_DIR}/custom.css")

# Commande pour copier le fichier custom.css dans le dossier de build
add_custom_command(
    OUTPUT ${CSS_DESTINATION}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CSS_SOURCE} ${CSS_DESTINATION}
    DEPENDS ${CSS_SOURCE}
    COMMENT "📁 Copie de custom.css dans le dossier de build"
)

# Ajoute une cible de build dédiée au CSS pour l’intégrer dans le build principal
add_custom_target(copy_css ALL DEPENDS ${CSS_DESTINATION})



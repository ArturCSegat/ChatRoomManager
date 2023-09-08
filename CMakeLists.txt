cmake_minimum_required(VERSION 3.0)

project(ChatRoomManager)

# Define source files
file(GLOB SERVER_SOURCE "src/server.c")
file(GLOB CLIENT_SOURCE "src/client.c")
file(GLOB MODULE_SOURCES "headers/modules/*.c")

# Include directories
include_directories(headers)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# Define the executable
add_executable(server ${SERVER_SOURCE} ${MODULE_SOURCES})
add_executable(client ${CLIENT_SOURCE} ${MODULE_SOURCES})


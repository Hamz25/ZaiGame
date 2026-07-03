cmake_minimum_required(VERSION 3.10)
project(ZaiGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use pre-built SFML 2.6
set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../SFML-2.6.0")
set(SFML_INCLUDE_DIR "${SFML_DIR}/include")
set(SFML_LIB_DIR "${SFML_DIR}/lib")

# Add source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Menu.cpp
    src/FlappyAI.cpp
    src/NeuralNetwork.cpp
)

# Add headers
set(HEADERS
    include/Game.h
    include/Menu.h
    include/FlappyAI.h
    include/NeuralNetwork.h
    include/Constants.h
)

# Create executable
add_executable(ZaiGame ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(ZaiGame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SFML_INCLUDE_DIR}
)

# Link SFML libraries
target_link_directories(ZaiGame PRIVATE ${SFML_LIB_DIR})
target_link_libraries(ZaiGame PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Post-build: copy assets and create saves folder
add_custom_command(TARGET ZaiGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/saves
    COMMENT "Copying assets and creating saves folder..."
)

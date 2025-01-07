cmake_minimum_required(VERSION 3.10)
project(ArtilleryGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Set source files
set(SOURCES
        src/main.cpp
        src/Game.cpp
        src/Tank.cpp
        src/Terrain.cpp
        src/Menu.cpp
)

# Set header files
set(HEADERS
        include/Game.h
        include/Tank.h
        include/Terrain.h
        include/Menu.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link SFML
target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

# Copy resources to build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})

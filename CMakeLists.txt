cmake_minimum_required(VERSION 3.5)

project(parula LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_executable(parula main.cpp
                       UI/window.cpp
                       Graphics/image_engine.cpp
                       Graphics/texture.cpp
                       Input/input_manager.cpp
                       Input/StylusInput/stylus.cpp
                       Modules/Recognition/rec_worker.cpp)

target_include_directories(parula PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(parula PRIVATE SDL2::SDL2 SDL2::SDL2main TBB::tbb)

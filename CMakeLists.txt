cmake_minimum_required(VERSION 3.5)

project(parula LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads)

include_directories(${Boost_INCLUDE_DIR})

add_executable(parula main.cpp
                       UI/window.cpp
                       Graphics/image_engine.cpp
                       Graphics/texture.cpp
                       Input/input_manager.cpp
                       Input/StylusInput/stylus.cpp
                       Modules/Recognition/rec_worker.cpp
                       Modules/DrawPath/draw_path.cpp)

target_include_directories(parula PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(parula PRIVATE SDL2::SDL2
                                     SDL2::SDL2main
                                     TBB::tbb
                                     Threads::Threads)


install(TARGETS parula DESTINATION bin)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${parula_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${parula_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${parula_VERSION_PATCH}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alexey")
include(CPack)

if (MSVC)
    target_compile_options(
        parula PRIVATE
        /W4
    )
    add_compile_definitions(_WIN32_WINNT=0x0601)
else ()
    target_compile_options(
        parula PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

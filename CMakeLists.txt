cmake_minimum_required(VERSION 3.10)
project(AsciiScreen CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Security: Enable compiler hardening flags
if(MSVC)
    # /GS: Buffer Security Check
    # /guard:cf: Control Flow Guard
    add_compile_options(/GS /guard:cf)
else()
    # GCC/Clang: Stack Smashing Protection
    add_compile_options(-fstack-protector-strong)
endif()

# Add screen_capture_lite from the lib directory
include_directories(lib/screen_capture_lite/include)

add_executable(AsciiScreen src/main.cpp)

# Platform-specific libraries needed by screen_capture_lite
if(WIN32)
    # For Windows, link against GDI and User libraries
    target_link_libraries(AsciiScreen PRIVATE gdi32 user32)
elseif(APPLE)
    # For macOS, link against CoreGraphics and CoreFoundation frameworks
    target_link_libraries(AsciiScreen PRIVATE "-framework CoreGraphics" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE)
    # For Linux, find and link against X11
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(AsciiScreen PRIVATE ${X11_LIBRARIES})
endif()
cmake_minimum_required(VERSION 3.21)
project(Ambient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Crea la libreria Tonic
file(GLOB TONIC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Tonic/*.cpp")
add_library(TonicLib STATIC ${TONIC_SOURCES})
target_include_directories(TonicLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(ambient
    src/main.cpp
    src/rtaudio/RtAudio.cpp
)

if(APPLE)
    # macOS (CoreAudio)
    target_compile_definitions(ambient PRIVATE __MACOSX_CORE__)
    target_link_libraries(ambient PRIVATE "-framework CoreAudio" "-framework CoreFoundation")

elseif(WIN32)
    # Windows (WASAPI)
    target_compile_definitions(ambient PRIVATE __WINDOWS_WASAPI__)
    target_link_libraries(ambient PRIVATE ole32 uuid ksuser)

else()
    # Linux (PULSE)
    target_compile_definitions(ambient PRIVATE __LINUX_PULSE__)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PULSE REQUIRED libpulse libpulse-simple)

    target_include_directories(ambient PRIVATE ${PULSE_INCLUDE_DIRS})
    target_link_libraries(ambient PRIVATE ${PULSE_LIBRARIES} pthread)
endif()

target_link_libraries(ambient PRIVATE TonicLib)

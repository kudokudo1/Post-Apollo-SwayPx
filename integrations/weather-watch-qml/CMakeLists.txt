cmake_minimum_required(VERSION 3.21)
project(PostApolloStageDirectorWatchBridge VERSION 0.1 LANGUAGES C CXX)

# The compositor's XML is the ONLY canonical copy. A mismatched client
# protocol must fail the build, not silently open another Wayland connection.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Gui Qml)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
find_program(WAYLAND_SCANNER NAMES wayland-scanner REQUIRED)

set(PROTOCOL_XML "${CMAKE_CURRENT_LIST_DIR}/../../protocols/post-apollo-weather-watch-v1.xml")
if(NOT EXISTS "${PROTOCOL_XML}")
    message(FATAL_ERROR "Missing SwayPX canonical WATCH v2 protocol")
endif()
set(GEN "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GEN}")
add_custom_command(
    OUTPUT "${GEN}/post-apollo-weather-watch-v1-client-protocol.h"
    COMMAND "${WAYLAND_SCANNER}" client-header "${PROTOCOL_XML}"
            "${GEN}/post-apollo-weather-watch-v1-client-protocol.h"
    DEPENDS "${PROTOCOL_XML}" VERBATIM)
add_custom_command(
    OUTPUT "${GEN}/post-apollo-weather-watch-v1-protocol.c"
    COMMAND "${WAYLAND_SCANNER}" private-code "${PROTOCOL_XML}"
            "${GEN}/post-apollo-weather-watch-v1-protocol.c"
    DEPENDS "${PROTOCOL_XML}" VERBATIM)
add_custom_target(weather_watch_protocol_header
    DEPENDS "${GEN}/post-apollo-weather-watch-v1-client-protocol.h")

qt_add_qml_module(postapollo_weather_watch
    URI PostApollo.StageDirector
    VERSION 1.0
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/StageDirector"
    SOURCES WatchBridge.hpp WatchBridge.cpp
        "${GEN}/post-apollo-weather-watch-v1-protocol.c"
)
add_dependencies(postapollo_weather_watch weather_watch_protocol_header)
target_include_directories(postapollo_weather_watch PRIVATE "${GEN}")
target_link_libraries(postapollo_weather_watch PRIVATE
    Qt6::Gui Qt6::Qml PkgConfig::WAYLAND_CLIENT)
target_compile_options(postapollo_weather_watch PRIVATE
    -Wall -Wextra -Werror -Wpedantic)

# Portable runtime artifact: the module may be installed into Quickshell's
# QML import path later, only with operator-approved local deployment.
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/"
    DESTINATION lib/qt6/qml
    FILES_MATCHING PATTERN "qmldir" PATTERN "*.so" PATTERN "*.qmltypes")

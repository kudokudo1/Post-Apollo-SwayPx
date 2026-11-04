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

enable_testing()
qt_add_executable(watch_qml_offscreen_test test_inactive_qml.cpp)
target_link_libraries(watch_qml_offscreen_test PRIVATE Qt6::Gui Qt6::Qml)
target_compile_options(watch_qml_offscreen_test PRIVATE -Wall -Wextra -Werror)
add_dependencies(watch_qml_offscreen_test postapollo_weather_watch)
add_test(NAME watch-bridge-disabled-by-default
    COMMAND watch_qml_offscreen_test "${CMAKE_CURRENT_BINARY_DIR}/qml")
set_tests_properties(watch-bridge-disabled-by-default PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Install backing library AND QML plugin side by side: the plugin must
# not retain a forbidden RPATH to the temporary Nix build directory.
set_target_properties(postapollo_weather_watch
    postapollo_weather_watchplugin PROPERTIES INSTALL_RPATH "$ORIGIN")
set(QML_MODULE_INSTALL "lib/qt6/qml/PostApollo/StageDirector")
install(TARGETS postapollo_weather_watch postapollo_weather_watchplugin
    LIBRARY DESTINATION "${QML_MODULE_INSTALL}"
    ARCHIVE DESTINATION "${QML_MODULE_INSTALL}")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/StageDirector/qmldir"
    "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/StageDirector/postapollo_weather_watch.qmltypes"
    DESTINATION "${QML_MODULE_INSTALL}")
# The smoke is executed against this installed package in bridge.nix.

cmake_minimum_required(VERSION 3.21)
project(PostApolloCompanyVectorBridge VERSION 0.1 LANGUAGES C CXX)

# Canonical SwayPX protocol XML, never a second independent protocol copy.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Gui Qml)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
find_program(WAYLAND_SCANNER NAMES wayland-scanner REQUIRED)

set(PROTOCOL_XML "${CMAKE_CURRENT_LIST_DIR}/../../protocols/post-apollo-company-vector-v1.xml")
if(NOT EXISTS "${PROTOCOL_XML}")
    message(FATAL_ERROR "Missing canonical SwayPX Company vector v1 protocol")
endif()

set(GEN "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GEN}")
add_custom_command(
    OUTPUT "${GEN}/post-apollo-company-vector-v1-client-protocol.h"
    COMMAND "${WAYLAND_SCANNER}" client-header "${PROTOCOL_XML}"
            "${GEN}/post-apollo-company-vector-v1-client-protocol.h"
    DEPENDS "${PROTOCOL_XML}" VERBATIM)
add_custom_command(
    OUTPUT "${GEN}/post-apollo-company-vector-v1-protocol.c"
    COMMAND "${WAYLAND_SCANNER}" private-code "${PROTOCOL_XML}"
            "${GEN}/post-apollo-company-vector-v1-protocol.c"
    DEPENDS "${PROTOCOL_XML}" VERBATIM)
add_custom_target(company_vector_protocol_header
    DEPENDS "${GEN}/post-apollo-company-vector-v1-client-protocol.h")

qt_add_qml_module(postapollo_company_vector
    URI PostApollo.CompanyVector
    VERSION 1.0
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/CompanyVector"
    SOURCES CompanyVectorBridge.hpp CompanyVectorBridge.cpp
        "${GEN}/post-apollo-company-vector-v1-protocol.c"
)
add_dependencies(postapollo_company_vector company_vector_protocol_header)
target_include_directories(postapollo_company_vector PRIVATE "${GEN}")
target_link_libraries(postapollo_company_vector PRIVATE
    Qt6::Gui Qt6::Qml PkgConfig::WAYLAND_CLIENT)
target_compile_options(postapollo_company_vector PRIVATE
    -Wall -Wextra -Werror -Wpedantic)

enable_testing()
qt_add_executable(company_vector_qml_disabled_test test_disabled_qml.cpp)
target_link_libraries(company_vector_qml_disabled_test PRIVATE
    Qt6::Gui Qt6::Qml)
target_compile_options(company_vector_qml_disabled_test PRIVATE
    -Wall -Wextra -Werror)
add_dependencies(company_vector_qml_disabled_test postapollo_company_vector)
add_test(NAME company-vector-bridge-disabled-by-default
    COMMAND company_vector_qml_disabled_test "${CMAKE_CURRENT_BINARY_DIR}/qml")
set_tests_properties(company-vector-bridge-disabled-by-default PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The installed QML module retains no RPATH to its build environment.
set_target_properties(postapollo_company_vector
    postapollo_company_vectorplugin PROPERTIES INSTALL_RPATH "$ORIGIN")
set(QML_MODULE_INSTALL "lib/qt6/qml/PostApollo/CompanyVector")
install(TARGETS postapollo_company_vector postapollo_company_vectorplugin
    LIBRARY DESTINATION "${QML_MODULE_INSTALL}"
    ARCHIVE DESTINATION "${QML_MODULE_INSTALL}")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/CompanyVector/qmldir"
    "${CMAKE_CURRENT_BINARY_DIR}/qml/PostApollo/CompanyVector/postapollo_company_vector.qmltypes"
    DESTINATION "${QML_MODULE_INSTALL}")

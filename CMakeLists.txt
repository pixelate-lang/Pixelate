cmake_minimum_required(VERSION 3.16)

project(pixelate LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PIXELATE_PXLDOC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/pxldocs/builtins.pxldoc
)
set(PIXELATE_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(PIXELATE_EMBEDDED_PXLDOC_HEADER
    ${PIXELATE_GENERATED_DIR}/pxldocs_embedded.h
)

add_custom_command(
    OUTPUT ${PIXELATE_EMBEDDED_PXLDOC_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PIXELATE_GENERATED_DIR}
    COMMAND ${CMAKE_COMMAND}
        -DINPUT=${PIXELATE_PXLDOC_SOURCE}
        -DOUTPUT=${PIXELATE_EMBEDDED_PXLDOC_HEADER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_pxldocs.cmake
    DEPENDS
        ${PIXELATE_PXLDOC_SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_pxldocs.cmake
    VERBATIM
)

set(PIXELATE_COMMON_SOURCES
    language/docs.cpp
    language/interpreter.cpp
    language/lexer.cpp
    language/parser/program.cpp
    language/parser/elements.cpp
    language/parser/updates.cpp
    language/parser/statements.cpp
    runtime/pxl_runtime.cpp
    graphics/ui/button.cpp
    graphics/ui/label.cpp
    graphics/ui/paragraph.cpp
    graphics/ui/textarea.cpp
    graphics/ui/textbox.cpp
    graphics/ui/widget.cpp
    graphics/ui/shape.cpp
    ${PIXELATE_EMBEDDED_PXLDOC_HEADER}
)

# This target deliberately contains only the platform-neutral implementation.
# It gives every platform a small, useful compilation target while the native
# window backend is being implemented.
add_library(pixelate_core STATIC ${PIXELATE_COMMON_SOURCES})
target_include_directories(pixelate_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PIXELATE_GENERATED_DIR}
)

if(WIN32)
    add_executable(pixelate
        tools/pxl_main.cpp
        graphics/platform/windows/win32_app.cpp
        graphics/platform/windows/win32_draw_context.cpp
        graphics/platform/windows/win32_native_textbox.cpp
        graphics/platform/windows/win32_native_textarea.cpp
        graphics/platform/windows/pixelate.rc
    )

    target_link_libraries(pixelate PRIVATE pixelate_core)
    target_include_directories(pixelate PRIVATE graphics)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

    add_executable(pixelate
        tools/pxl_main.cpp
        graphics/platform/linux/gtk3_app.cpp
        graphics/platform/linux/gtk3_draw_context.cpp
        graphics/platform/linux/gtk3_native_textbox.cpp
        graphics/platform/linux/gtk3_native_textarea.cpp
    )

    target_link_libraries(pixelate PRIVATE pixelate_core PkgConfig::GTK3)
    target_include_directories(pixelate PRIVATE graphics)
endif()

if(BUILD_TESTING)
    add_executable(pixelate_parser_smoke
        tests/parser_smoke.cpp
        language/interpreter.cpp
        language/lexer.cpp
        language/parser/program.cpp
        language/parser/elements.cpp
        language/parser/updates.cpp
        language/parser/statements.cpp
    )
    target_include_directories(pixelate_parser_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(
        NAME pixelate_parse_demos
        COMMAND pixelate_parser_smoke
            "${CMAKE_CURRENT_SOURCE_DIR}/programs/demos2"
    )
endif()

install(TARGETS pixelate RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(
    DIRECTORY programs/demos2/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixelate/programs/demos
    FILES_MATCHING PATTERN "*.pxl"
)
install(
    DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/reference
    FILES_MATCHING PATTERN "*.md"
)
install(
    DIRECTORY pxldocs/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixelate/pxldocs
    FILES_MATCHING PATTERN "*.pxldoc"
)
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})


# Common library - shared code between monitor and viewer

add_library(bigbrother_common INTERFACE)

target_include_directories(bigbrother_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Header-only library for now
target_sources(bigbrother_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/session_logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/session_data.h
    ${CMAKE_CURRENT_SOURCE_DIR}/time_utils.h
)

# Link Windows libraries
target_link_libraries(bigbrother_common INTERFACE
    user32
    psapi
    shell32
)

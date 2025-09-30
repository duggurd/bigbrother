# BigBrother Viewer - ImGui GUI application

# Find ImGui (must be set up first)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp")
    message(WARNING "ImGui not found. Run setup_imgui.bat first.")
    return()
endif()

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Viewer executable
add_executable(bigbrother_viewer
    main.cpp
    ui/main_window.cpp
    ui/settings_window.cpp
    ui/timeline_view.cpp
    data/session_loader.cpp
    data/filter_manager.cpp
    graphics/icon_manager.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(bigbrother_viewer PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bigbrother_viewer PRIVATE
    bigbrother_common
    d3d11
    d3dcompiler
    dxgi
    gdi32
    ole32
)

# Set output name
set_target_properties(bigbrother_viewer PROPERTIES
    OUTPUT_NAME "bigbrother_viewer"
)

# Windows subsystem
if(MSVC)
    set_target_properties(bigbrother_viewer PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

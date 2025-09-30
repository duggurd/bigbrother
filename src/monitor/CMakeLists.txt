# BigBrother Monitor - CLI application

add_executable(bigbrother_monitor
    main.cpp
)

target_link_libraries(bigbrother_monitor PRIVATE
    bigbrother_common
)

# Set output name
set_target_properties(bigbrother_monitor PROPERTIES
    OUTPUT_NAME "bigbrother_monitor"
)

# Windows subsystem
if(MSVC)
    set_target_properties(bigbrother_monitor PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

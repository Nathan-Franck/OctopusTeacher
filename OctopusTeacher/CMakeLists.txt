set (SOURCE_FILES
        main.cpp
        stdafx.cpp
        OctopusTeacher.rc
        ../Editor/Translator.cpp
    )

add_executable(OctopusTeacher WIN32 ${SOURCE_FILES})

target_link_libraries(OctopusTeacher PUBLIC
        WickedEngine_Windows
    )

set_property(TARGET OctopusTeacher PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

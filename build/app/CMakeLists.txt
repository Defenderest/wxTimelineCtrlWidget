# wxTimelineCtrl Sample Application CMakeLists.txt

# Define application source files
set(WXTIMELINECTRL_APP_SOURCES
    ${CMAKE_SOURCE_DIR}/../app/wxTimelineCtrlApp.cpp
    ${CMAKE_SOURCE_DIR}/../app/wxTimelineCtrlApp.h
    ${CMAKE_SOURCE_DIR}/../app/wxTimelineCtrlTestMainFrame.cpp
    ${CMAKE_SOURCE_DIR}/../app/wxTimelineCtrlTestMainFrame.h
    ${CMAKE_SOURCE_DIR}/../app/SampleData.h
)

# Create the executable
add_executable(wxTimelineCtrl_App ${WXTIMELINECTRL_APP_SOURCES})

# Link against the library
target_link_libraries(wxTimelineCtrl_App 
    PRIVATE 
    wxTimelineCtrl_Lib
    ${wxWidgets_LIBRARIES}
)

# Set target properties
target_include_directories(wxTimelineCtrl_App PRIVATE
    ${CMAKE_SOURCE_DIR}/../lib
    ${CMAKE_SOURCE_DIR}/../app
    ${wxWidgets_INCLUDE_DIRS}
)

target_compile_definitions(wxTimelineCtrl_App PRIVATE
    ${wxWidgets_DEFINITIONS}
)

# Set compile features
target_compile_features(wxTimelineCtrl_App PRIVATE 
    cxx_std_17
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(wxTimelineCtrl_App PRIVATE
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
    )
    
    # Set the executable to be a Windows application (not console)
    set_target_properties(wxTimelineCtrl_App PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # For debug builds, allow console output
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(wxTimelineCtrl_App PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        )
    endif()
endif()

# Set output name
set_target_properties(wxTimelineCtrl_App PROPERTIES
    OUTPUT_NAME "wxTimelineCtrlTest"
)

# Additional compiler flags
if(MSVC)
    target_compile_options(wxTimelineCtrl_App PRIVATE
        /W4
        /wd4996  # Disable deprecated warnings
    )
    
    # Enable precompiled headers for faster compilation
    target_precompile_headers(wxTimelineCtrl_App PRIVATE
        <wx/wxprec.h>
        <wx/wx.h>
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(wxTimelineCtrl_App PRIVATE
        -Wall 
        -Wextra 
        -pedantic
        -Wno-unused-parameter
    )
endif()

# Set debug/release specific settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(wxTimelineCtrl_App PRIVATE
        _DEBUG
        DEBUG
    )
else()
    target_compile_definitions(wxTimelineCtrl_App PRIVATE
        NDEBUG
    )
endif()

# Install rules for the application
install(TARGETS wxTimelineCtrl_App
    RUNTIME DESTINATION bin
    COMPONENT Application
)

# Create a desktop shortcut on Windows (optional)
if(WIN32)
    # You can add Windows-specific installation items here
    # install(FILES "${CMAKE_SOURCE_DIR}/wxTimelineCtrlTest.ico" DESTINATION bin OPTIONAL)
endif()

message(STATUS "wxTimelineCtrl sample application configured")

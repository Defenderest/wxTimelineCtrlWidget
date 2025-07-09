# wxTimelineCtrl Library CMakeLists.txt

# Define library source files
set(WXTIMELINECTRL_SOURCES
    # Header files
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl.h
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl_impl.h
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl.inl
    ${CMAKE_SOURCE_DIR}/../lib/TimelineArtProvider.h
    ${CMAKE_SOURCE_DIR}/../lib/TimelineArtProvider.inl
    ${CMAKE_SOURCE_DIR}/../lib/TimelineItem.h
    ${CMAKE_SOURCE_DIR}/../lib/TimelineItemData.h
    ${CMAKE_SOURCE_DIR}/../lib/FloatingItemPopupWindow.h
)

# Create a header-only library target
add_library(wxTimelineCtrl_Lib INTERFACE)

# Set target properties
target_include_directories(wxTimelineCtrl_Lib INTERFACE
    ${CMAKE_SOURCE_DIR}/../lib
    ${CMAKE_SOURCE_DIR}/../app
    ${wxWidgets_INCLUDE_DIRS}
)

target_link_libraries(wxTimelineCtrl_Lib INTERFACE
    ${wxWidgets_LIBRARIES}
)

target_compile_definitions(wxTimelineCtrl_Lib INTERFACE
    ${wxWidgets_DEFINITIONS}
)

# Set compile features
target_compile_features(wxTimelineCtrl_Lib INTERFACE 
    cxx_std_17
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(wxTimelineCtrl_Lib INTERFACE
        WIN32
        _WINDOWS
        UNICODE
        _UNICODE
    )
endif()

# Install library headers
install(FILES
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl.h
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl_impl.h
    ${CMAKE_SOURCE_DIR}/../lib/wxTimelineCtrl.inl
    ${CMAKE_SOURCE_DIR}/../lib/TimelineArtProvider.h
    ${CMAKE_SOURCE_DIR}/../lib/TimelineArtProvider.inl
    ${CMAKE_SOURCE_DIR}/../lib/TimelineItem.h
    ${CMAKE_SOURCE_DIR}/../lib/TimelineItemData.h
    ${CMAKE_SOURCE_DIR}/../lib/FloatingItemPopupWindow.h
    DESTINATION include/wxTimelineCtrl
)

# Create a custom target for IDE support (shows headers in project)
add_custom_target(wxTimelineCtrl_Headers SOURCES ${WXTIMELINECTRL_SOURCES})

message(STATUS "wxTimelineCtrl library configured as header-only interface library")

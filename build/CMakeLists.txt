cmake_minimum_required(VERSION 3.16)

# Project configuration
project(wxTimelineCtrl 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "A wxWidgets timeline control library"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories based on configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# For multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${CONFIG})
endforeach()

# Find wxWidgets
set(wxWidgets_ROOT_DIR "$ENV{WXWIN}")
if(NOT wxWidgets_ROOT_DIR)
    message(FATAL_ERROR "WXWIN environment variable not set. Please set it to your wxWidgets installation directory.")
endif()

find_package(wxWidgets REQUIRED COMPONENTS core base adv)

if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
    message(STATUS "Found wxWidgets: ${wxWidgets_VERSION}")
    message(STATUS "wxWidgets_ROOT_DIR: ${wxWidgets_ROOT_DIR}")
    message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")
    message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "wxWidgets not found!")
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    add_compile_definitions(WIN32 _WINDOWS)
    set(CMAKE_WIN32_EXECUTABLE ON)
    
    # Set subsystem for release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    endif()
endif()

# Compiler-specific settings
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Enable Unicode
    add_compile_definitions(UNICODE _UNICODE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/../lib)
include_directories(${CMAKE_SOURCE_DIR}/../app)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(app)

# Install rules
install(TARGETS wxTimelineCtrl_App
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Create a simple test target
enable_testing()
add_test(NAME wxTimelineCtrl_Basic_Test 
    COMMAND wxTimelineCtrl_App --help
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Print configuration summary
message(STATUS "")
message(STATUS "=== wxTimelineCtrl Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "wxWidgets version: ${wxWidgets_VERSION}")
message(STATUS "===========================================")
message(STATUS "")

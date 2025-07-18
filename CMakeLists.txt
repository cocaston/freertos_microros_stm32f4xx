cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME freertos_microros_led)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)



# Set library variables
set(APP_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/App)
set(MICRO_ROS_LIB_BASE_DIR ${APP_BASE_DIR}/lib/micro_ros)
set(MICRO_ROS_STATIC_LIB_DIR ${MICRO_ROS_LIB_BASE_DIR}/static_library)
set(MICRO_ROS_EXTRA_SOURCES_DIR ${MICRO_ROS_LIB_BASE_DIR}/extra_sources)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${MICRO_ROS_EXTRA_SOURCES_DIR}/custom_memory_manager.c
    ${MICRO_ROS_EXTRA_SOURCES_DIR}/dma_transport.c
    ${MICRO_ROS_EXTRA_SOURCES_DIR}/microros_allocators.c
    ${MICRO_ROS_EXTRA_SOURCES_DIR}/microros_time.c
    ${APP_BASE_DIR}/Core/Src/app.c
)

# Add include paths
file(GLOB MICRO_ROS_STATIC_LIB_ALL_ITEMS
    LIST_DIRECTORIES true
    RELATIVE
    ${MICRO_ROS_STATIC_LIB_DIR}/include
    ${MICRO_ROS_STATIC_LIB_DIR}/include/*
)

set(MICRO_ROS_SUB_DIRECTORIES)

foreach(item ${MICRO_ROS_STATIC_LIB_ALL_ITEMS})
    if(IS_DIRECTORY "${MICRO_ROS_STATIC_LIB_DIR}/include/${item}")
        list(APPEND MICRO_ROS_SUB_DIRECTORIES "${MICRO_ROS_STATIC_LIB_DIR}/include/${item}")
    endif()
endforeach()



target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${MICRO_ROS_STATIC_LIB_DIR}/include
    ${MICRO_ROS_SUB_DIRECTORIES}
    ${APP_BASE_DIR}/Core/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
    ${MICRO_ROS_STATIC_LIB_DIR}/libmicroros.a
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating binary file from ELF"
)

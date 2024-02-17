project("FreeRTOS BSP for F&S boards")

cmake_minimum_required(VERSION 3.0)

### Project and build paths ###

# Set path values

set(EXAMPLE_DIR boards/${SOC})
set(BUILD_DIR ${PROJECT_SOURCE_DIR}/build/${BOARD})

### Defines for preprocessor ###

string(TOUPPER ${BOARD} board_define)
add_definitions(-DBOARD_${board_define})
set(${board_define} ON CACHE BOOL "The chosen board type." FORCE)

### CMake magic goes here ###

# Since SDK 2.9.1 the structure of the CMakeLists.txt files changed
# significantly. All necessary includes are now split up and
# spread across several modules.Unlike before, where all of the inclusion
# happened in the CMakeLists.txt file of the respective example, now the
# main task of these files is to include the modules which then execute
# their CMake code consisting of the required include commands.

# Since these examples are meant to be built independently from each other
# some problems occur if the build-process is kicked off
# from the project root directory. One possible solution
# is to force set the module variables in the CMakeCache.txt to false.
# This forces CMake to load every module for every example every single time.

# Search for modules. All modules have the same ending
file(GLOB_RECURSE module_path ${PROJECT_SOURCE_DIR}/*_MIMX8ML8.cmake)

# Naming convention for the module variables: MODULE_NAME_IN_UPPERCASE_INCLUDED
foreach (module_dir_name IN LISTS module_path)

# Strip the filename from the path
        get_filename_component(module_file_name ${module_dir_name} NAME)

# delete the file name extension ".cmake"
        string(REGEX REPLACE ".cmake" "" module_name "${module_file_name}")

        string(TOUPPER ${module_name} module)
        string(APPEND module "_INCLUDED")

# set() command to save the variable in CMakeCache.txt
        set(${module} OFF CACHE BOOL "" FORCE)

endforeach()

# convert Release to lower case release
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)

# Search for build scripts in subdir
file(GLOB_RECURSE examples FOLLOW_SYMLINKS  ${PROJECT_SOURCE_DIR}/${EXAMPLE_DIR}/*build_${BUILD_TYPE_LOWER}.sh)

### Install the binaries from build/ to a bin/ directory ###

# Set install path
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

install(CODE "file(GLOB_RECURSE binaries \"${BUILD_DIR}/*.bin\")")
install(CODE "file(INSTALL \${binaries}
                   DESTINATION ${CMAKE_INSTALL_PREFIX}/output/${BOARD}/bin
                   USE_SOURCE_PERMISSIONS
                   FILES_MATCHING PATTERN *.bin)")

install(CODE "file(GLOB_RECURSE elfs \"${BUILD_DIR}/*.elf\")")
install(CODE "file(INSTALL \${elfs}
                   DESTINATION ${CMAKE_INSTALL_PREFIX}/output/${BOARD}/elf
                   USE_SOURCE_PERMISSIONS
                   FILES_MATCHING PATTERN *.elf)")



### Clean ###

add_custom_target(clean-all
                  COMMAND ${CMAKE_BUILD_TOOL} clean
                  COMMAND ${CMAKE_COMMAND} -P clean_all.cmake)

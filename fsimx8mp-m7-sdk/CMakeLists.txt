project("FreeRTOS BSP for F&S boards")

cmake_minimum_required(VERSION 3.0)

### Project and build paths ###

# Set path values

set(EXAMPLE_DIR examples/${SOC})

set(BUILD_DIR ${PROJECT_SOURCE_DIR}/build/${BOARD})


### Defines for preprocessor ###

### Set Board for stuff like shared_clock management
##if (${BOARD} EQUAL efusa9x)
##  add_definitions(-DBOARD_EFUSA9X)
##  message(STATUS "Enable flexcan2 shared clock for efusA9X")
##elseif (${BOARD} EQUAL picocoma9x)
##  add_definitions(-DBOARD_PICOCOMA9X)
##  message(STATUS "Enable flexcan1 shared clock for picocomA9X")
##endif()

# Overwrite defines for RPMsg and stuff
set(_DEFINITIONS -DVRING0_BASE=${VRING0_BASE}
                 -DVRING1_BASE=${VRING1_BASE})
add_definitions(${_DEFINITIONS})

# Set cache variables to customize the build process
if (${PCB_VERSION} MATCHES 1)
        add_definitions(-DPICOCOREMX8MM)
        set(PICOCOREMX8MM ON CACHE BOOL "The chosen module variant." FORCE)
elseif (${PCB_VERSION} MATCHES 2)
        add_definitions(-DPICOCOREMX8MX)
        set(PICOCOREMX8MX ON CACHE BOOL "The chosen module variant." FORCE)
endif()

# Add define for PicoCOMA9X with i.MX6SX < 1504
# to prevent the CPUs from hangup
if (${WORKAROUND} MATCHES 1)
  add_definitions(-DWORKAROUND)
  message(STATUS "Use workaround for pico")
endif (${WORKAROUND} MATCHES 1)

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
file(GLOB_RECURSE module_path ${PROJECT_SOURCE_DIR}/*_MIMX8MM6.cmake)

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

# Truncate path and add each subproject to the build process
foreach(example IN LISTS examples)
  string(REGEX REPLACE "/build_${BUILD_TYPE_LOWER}.sh" "" example_path ${example})
  string(REGEX MATCH "\\/([^\\/]+)\\/([^\\/]+)\\/([^\\/]+)\\/([^\\/]+)\\/([^\\/]+)\\/([^\\/]+)\\/armgcc" project_name ${example})

#checking the required subfolder count and adding them
if(${CMAKE_MATCH_1} STREQUAL examples)
  add_subdirectory(${example_path} ${BUILD_DIR}/${CMAKE_MATCH_3}/${CMAKE_MATCH_4}/${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
  message(STATUS "Example added: " ${CMAKE_MATCH_3}/${CMAKE_MATCH_4}/${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
elseif(${CMAKE_MATCH_2} STREQUAL examples)
  add_subdirectory(${example_path} ${BUILD_DIR}/${CMAKE_MATCH_4}/${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
  message(STATUS "Example added: " ${CMAKE_MATCH_4}/${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
elseif(${CMAKE_MATCH_3} STREQUAL examples)
  add_subdirectory(${example_path} ${BUILD_DIR}/${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
  message(STATUS "Example added: " ${CMAKE_MATCH_5}/${CMAKE_MATCH_6})
elseif(${CMAKE_MATCH_4} STREQUAL examples)
  add_subdirectory(${example_path} ${BUILD_DIR}/${CMAKE_MATCH_6})
  message(STATUS "Example added: " ${CMAKE_MATCH_6})
endif()
endforeach(example)

### Install the binaries from build/ to a bin/ directory ###

# Set install path
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

install(CODE "file(GLOB_RECURSE binaries \"${BUILD_DIR}/*.bin\")")
install(CODE "file(INSTALL \${binaries}
                   DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${BOARD}
                   USE_SOURCE_PERMISSIONS
                   FILES_MATCHING PATTERN *.bin)")

if (${BOARD} MATCHES picocoremx7ulp)
	install(CODE "execute_process(COMMAND tools/imgutil/install7ulp.sh)")
endif (${BOARD} MATCHES picocoremx7ulp)

### Clean ###

add_custom_target(clean-all
                  COMMAND ${CMAKE_BUILD_TOOL} clean
                  COMMAND ${CMAKE_COMMAND} -P clean_all.cmake)

add_custom_target(images
                  COMMAND tools/imgutil/install7ulp.sh)

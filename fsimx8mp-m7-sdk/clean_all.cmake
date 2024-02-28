set(cmake_generated ${CMAKE_BINARY_DIR}/CMakeCache.txt
                    ${CMAKE_BINARY_DIR}/cmake_install.cmake
                    ${CMAKE_BINARY_DIR}/install_manifest.txt
                    ${CMAKE_BINARY_DIR}/Makefile
                    ${CMAKE_BINARY_DIR}/CMakeFiles
                    ${CMAKE_BINARY_DIR}/bin
                    ${CMAKE_BINARY_DIR}/build
)

foreach(file ${cmake_generated})
  message(STATUS "Removed: " ${file})
  if (EXISTS ${file})
    file(REMOVE_RECURSE ${file})
  endif()
endforeach(file)

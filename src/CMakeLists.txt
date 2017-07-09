project(pool-serial CXX)
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# Tell CMake where to find modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Download and unpack googletest at configure time.
configure_file(CMakeLists.txt.in
               googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
 
# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)
 
# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include"
                      "${gmock_SOURCE_DIR}/include")
endif()

add_subdirectory(../third-party/Simple-Web-Server ${CMAKE_BINARY_DIR}/Simple-Web-Server)

add_definitions(-DELPP_THREAD_SAFE)
include_directories(../third-party/kaitai_struct_cpp_stl_runtime)
add_executable(example serial.cpp easylogging++.cc pentair.cpp ../third-party/kaitai_struct_cpp_stl_runtime/kaitai/kaitaistream.cpp)
#add_dependencies(example kaitai)
target_link_libraries(example z pthread)

#add_test(NAME example_test COMMAND example)
set_property(TARGET example PROPERTY CXX_STANDARD 11)
set_property(TARGET example PROPERTY CXX_STANDARD_REQUIRED ON)

FIND_PACKAGE(Boost)
find_package(Boost 1.45.0 COMPONENTS timer system thread system)

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
    target_link_libraries(example ${Boost_LIBRARIES})
ENDIF()



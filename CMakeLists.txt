# Set the minimum CMake version required to build the project.
cmake_minimum_required( VERSION 3.1 )

# Set the project's name and version.
project( th2hdf5 VERSION 1.0 )

# Set the warning flag(s) to use.
set( CMAKE_CXX_STANDARD 14)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )

include_directories(include src)

find_package( HDF5 1.10 REQUIRED COMPONENTS CXX HL)
find_package(ROOT REQUIRED COMPONENTS RIO Hist Tree Net Core)
find_package(ZLIB)

# CMake often finds the wrong include directories (i.e. on lxplus)
include_directories(BEFORE SYSTEM ${HDF5_INCLUDE_DIRS})

add_executable(th2hdf5
  src/th2hdf5.cxx
  src/convert_tree.cxx
  src/unshittify.cxx)

target_include_directories( th2hdf5
  PRIVATE ${HDF5_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(th2hdf5 ${HDF5_LIBRARIES} ${ROOT_LIBRARIES})

install(TARGETS th2hdf5
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include)

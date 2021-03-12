cmake_minimum_required(VERSION 3.13)

# cmake modules
include(ExternalProject)

# set the project name
project(hashgraph)

# compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g -Wall -DWRITE_LOG") # -DDEBUG -DMAKE_FORKS

# download and build thrift lib and compiler
ExternalProject_Add(thrift_project
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/thrift
  PREFIX ${PROJECT_SOURCE_DIR}/external

  # set install path
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external

  # make a shared lib
  # CMAKE_ARGS -DBUILD_SHARED_LIBS=ON

  # disable unnecessary libraries
  CMAKE_ARGS -DWITH_AS3:BOOL=OFF
  CMAKE_ARGS -DWITH_C_GLIB:BOOL=OFF
  CMAKE_ARGS -DWITH_NODEJS:BOOL=OFF
  CMAKE_ARGS -DWITH_JAVASCRIPT:BOOL=OFF
  CMAKE_ARGS -DWITH_PYTHON:BOOL=OFF
  CMAKE_ARGS -DWITH_HASKELL:BOOL=OFF
)

# generate thrift interfaces and messages
add_custom_command(
    OUTPUT 
      ${PROJECT_SOURCE_DIR}/src/message/Gossip.cpp 
      ${PROJECT_SOURCE_DIR}/src/message/hashgraph_types.cpp
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/def
    DEPENDS thrift_project
    
    COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/src/message
    COMMAND ${PROJECT_SOURCE_DIR}/external/bin/thrift -r -gen cpp -out ${PROJECT_SOURCE_DIR}/src/message hashgraph.thrift
)

# set hashgraph executable
add_executable(hashgraph 
  src/main.cpp
  src/Runner.cpp
  src/message/Gossip.cpp
  src/message/hashgraph_types.cpp
  src/protocol/Event.cpp
  src/protocol/Person.cpp
  src/protocol/PersonNetworker.cpp
  src/utils/md5.cpp
)

# make thrift project required to build hashgraph
add_dependencies(hashgraph 
  thrift_project
)

# external includes
target_include_directories(hashgraph
  PUBLIC ${PROJECT_SOURCE_DIR}/external/include
)

# external libraries
target_link_directories(hashgraph 
  PUBLIC ${PROJECT_SOURCE_DIR}/external/lib
)

# link libraries
target_link_libraries(hashgraph 
  PUBLIC pthread thriftd
)

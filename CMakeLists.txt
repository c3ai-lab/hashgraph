cmake_minimum_required(VERSION 3.13)

# cmake modules
include(ExternalProject)

# set the project name
project(hashgraph CXX)

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
  CMAKE_ARGS -DWITH_JAVA:BOOL=OFF
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

# test target
add_custom_target(openssl_samples
  SOURCES 
    tests/keys/client.cert
    tests/keys/client.csr
    tests/keys/client.key
    tests/keys/server.cert
    tests/keys/server.key
)

# generated keys and certificates for testing
add_custom_command(
  OUTPUT 
    ${PROJECT_SOURCE_DIR}/tests/keys/client.cert
    ${PROJECT_SOURCE_DIR}/tests/keys/client.csr
    ${PROJECT_SOURCE_DIR}/tests/keys/client.key
    ${PROJECT_SOURCE_DIR}/tests/keys/server.cert
    ${PROJECT_SOURCE_DIR}/tests/keys/server.key

  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
  COMMAND ${PROJECT_SOURCE_DIR}/tests/scripts/openssl.sh
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
  src/types/Endpoint.cpp
  src/utils/sha384.cpp
  src/utils/ecdsa.cpp
)

# hashgraph dependencies
add_dependencies(hashgraph 
  thrift_project
  openssl_samples
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
  PUBLIC thriftd pthread ssl crypto
)

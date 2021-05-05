cmake_minimum_required(VERSION 3.13)

# set the project name
project(hashgraph CXX)

# append module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# cmake modules
include(Yaml)
include(Thrift)
include(Crypto)

# compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-g -Wall -DWRITE_LOG") # -DDEBUG -DMAKE_FORKS

# set hashgraph executable
add_executable(hashgraph 
  src/main.cpp
  src/Runner.cpp
  src/message/Gossip.cpp
  src/message/hashgraph_types.cpp
  src/protocol/Event.cpp
  src/protocol/Person.cpp
  src/protocol/PersonNetworker.cpp
  src/types/User.cpp
  src/types/Endpoint.cpp
  src/utils/sha384.cpp
  src/utils/ecdsa.cpp
)

# hashgraph dependencies
add_dependencies(hashgraph
  yaml_project
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
  PUBLIC yaml-cpp thriftd pthread ssl crypto
)

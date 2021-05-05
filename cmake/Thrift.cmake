# cmake modules
include(ExternalProject)

# download and build thrift lib and compiler
ExternalProject_Add(thrift_project
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/thrift
  PREFIX ${PROJECT_SOURCE_DIR}/external

  # configure step
  CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external
              # -DBUILD_SHARED_LIBS=ON
              -DWITH_AS3:BOOL=OFF
              -DWITH_C_GLIB:BOOL=OFF
              -DWITH_NODEJS:BOOL=OFF
              -DWITH_JAVASCRIPT:BOOL=OFF
              -DWITH_PYTHON:BOOL=OFF
              -DWITH_HASKELL:BOOL=OFF
              -DWITH_JAVA:BOOL=OFF
              -DBUILD_TESTING:BOOL=OFF
              -DBUILD_TUTORIALS:BOOL=OFF
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

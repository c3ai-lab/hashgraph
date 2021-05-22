# cmake modules
include(ExternalProject)

# download and build thrift lib and compiler
ExternalProject_Add(yaml_project
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/yaml-cpp
    PREFIX ${PROJECT_SOURCE_DIR}/external

    # configure step
    CMAKE_ARGS  
        -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external
        # -DBUILD_SHARED_LIBS=ON
        -DYAML_CPP_BUILD_TESTS:BOOL=OFF
)

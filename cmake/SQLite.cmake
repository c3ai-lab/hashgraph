# cmake modules
include(ExternalProject)

# dummy cmake file for sqlite
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/tmp)
file(WRITE ${CMAKE_SOURCE_DIR}/external/tmp/SQLite-CMakeLists.txt
    "cmake_minimum_required(VERSION 3.13)\n"
    "project(sqlite C)\n"
    "include_directories(${CMAKE_SOURCE_DIR}/external/src)\n"
    "add_library(sqlite3 ${CMAKE_SOURCE_DIR}/external/src/sqlite_project/sqlite3.c)\n"
    "install(TARGETS sqlite3 DESTINATION lib)\n"
    "install(FILES sqlite3.h DESTINATION include)\n")

# download, configure, build and install
ExternalProject_Add(sqlite_project
    URL https://www.sqlite.org/2021/sqlite-amalgamation-3350400.zip
    PREFIX ${PROJECT_SOURCE_DIR}/external

    # copy dummy file
    PATCH_COMMAND   
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/external/tmp/SQLite-CMakeLists.txt
        ${CMAKE_SOURCE_DIR}/external/src/sqlite_project/CMakeLists.txt

    # configure step
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_SOURCE_DIR}/external
)
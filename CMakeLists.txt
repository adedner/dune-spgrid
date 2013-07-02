project("dune-spgrid" C CXX)
cmake_minimum_required(VERSION 2.8.6)

find_package(dune-common)
list(APPEND CMAKE_MODULE_PATH ${dune-common_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

include(DuneMacros)
dune_project()

add_subdirectory(cmake/modules)
add_subdirectory(doc)
add_subdirectory(dune)
add_subdirectory(m4)

configure_file(${PROJECT_SOURCE_DIR}/dune/grid/spgrid/version.hh.in
               ${PROJECT_BINARY_DIR}/dune/grid/spgrid/version.hh
               @ONLY)

finalize_dune_project(GENERATE_CONFIG_H_CMAKE)
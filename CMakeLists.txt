cmake_minimum_required(VERSION 3.5)

project(xml_loader LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

add_subdirectory(src/xml_server)

add_subdirectory(src/tree_view_client)

cmake_minimum_required(VERSION 3.5)

project(tree_view_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)  # 14 - Помнить про Астру
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS
  Widgets
)

set(PROJECT_SOURCES
  main.cpp
  widget.cpp
  widget.h
)

add_executable(tree_view_client
  ${PROJECT_SOURCES}
)

target_link_libraries(tree_view_client PRIVATE
  Qt5::Widgets
)

install(TARGETS tree_view_client
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

cmake_minimum_required(VERSION 3.14)

project(xml_server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)  # 14 - Помнить про версию
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS
  Core
  Network
)

set(PROJECT_SOURCES
  ../shared/messages.h
  app_complete.cpp
  app_complete.h
  data_server.cpp
  data_server.h
  main.cpp
  tcp_server.cpp
  tcp_server.h
)

add_executable(xml_server
  ${PROJECT_SOURCES}
)

target_link_libraries(xml_server
  Qt5::Core
  Qt5::Network
)

install(TARGETS xml_server
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

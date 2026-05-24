cmake_minimum_required(VERSION 3.5)
project(
  spraybus
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(quill REQUIRED)
find_package(enet REQUIRED)

add_library(spraybus-networking src/spraybus/networking/networking.cpp)
target_include_directories(spraybus-networking
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(spraybus-networking PUBLIC enet::enet)

add_library(spraybus-client src/spraybus/client/client.cpp)
target_include_directories(spraybus-client PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(spraybus-client PUBLIC spraybus-networking)

add_library(spraybus-server src/spraybus/server/server.cpp
                            src/spraybus/server/logging.cpp)
target_include_directories(spraybus-server PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(spraybus-server PUBLIC spraybus-networking quill::quill)

add_executable(main drivers/main.cpp)
target_link_libraries(main PRIVATE spraybus-server)

add_executable(cli drivers/cli.cpp)
target_link_libraries(cli PRIVATE spraybus-client)

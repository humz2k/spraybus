cmake_minimum_required(VERSION 3.5)
project(
  spraybus
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(quill REQUIRED)
find_package(Boost REQUIRED)
find_package(enet REQUIRED)
find_package(prometheus-cpp REQUIRED)

add_library(spraybus)
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
target_sources(spraybus PRIVATE ${SOURCE_FILES})
target_include_directories(spraybus PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(spraybus PUBLIC quill::quill boost::boost enet::enet
                                      prometheus-cpp::prometheus-cpp)

file(GLOB DRIVERS_SOURCES ${PROJECT_SOURCE_DIR}/drivers/*.cpp)
foreach(sourcefile ${DRIVERS_SOURCES})
  get_filename_component(name ${sourcefile} NAME_WE)
  add_executable(${name} ${sourcefile})
  target_link_libraries(${name} spraybus)
endforeach(sourcefile ${DRIVERS_SOURCES})

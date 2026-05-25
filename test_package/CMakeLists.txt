cmake_minimum_required(VERSION 3.15)
project(spraybus_client_test_package LANGUAGES CXX)

find_package(spraybus-client CONFIG REQUIRED)

add_executable(example src/example.cpp)
target_compile_features(example PRIVATE cxx_std_23)
target_link_libraries(example PRIVATE spraybus::client)

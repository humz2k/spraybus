cmake_minimum_required(VERSION 3.15)
project(spraybus_client_example LANGUAGES CXX)

find_package(spraybus-client CONFIG REQUIRED)

add_executable(spraybus-client-example src/main.cpp)
target_compile_features(spraybus-client-example PRIVATE cxx_std_23)
target_link_libraries(spraybus-client-example PRIVATE spraybus::client)

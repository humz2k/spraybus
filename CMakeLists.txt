cmake_minimum_required(VERSION 3.15)
project(
  spraybus
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(SPRAYBUS_BUILD_SERVER "Build the spraybus server library" ON)
option(SPRAYBUS_BUILD_APPS "Build spraybus command-line applications" ON)
option(SPRAYBUS_BUILD_PYTHON "Build the spraybus Python extension module" ON)
option(SPRAYBUS_INSTALL_CLIENT "Install the spraybus client package" ON)

find_package(enet REQUIRED)

if(SPRAYBUS_BUILD_SERVER)
  find_package(quill REQUIRED)
endif()

function(spraybus_configure_library target)
  target_compile_features(${target} PUBLIC cxx_std_23)
  target_include_directories(
    ${target} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                     $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endfunction()

add_library(spraybus-networking src/spraybus/networking/networking.cpp)
add_library(spraybus::networking ALIAS spraybus-networking)
spraybus_configure_library(spraybus-networking)
target_link_libraries(spraybus-networking PUBLIC enet::enet)
set_target_properties(spraybus-networking PROPERTIES EXPORT_NAME networking)

add_library(spraybus-client src/spraybus/client/client.cpp)
add_library(spraybus::client ALIAS spraybus-client)
spraybus_configure_library(spraybus-client)
target_link_libraries(spraybus-client PUBLIC spraybus-networking)
set_target_properties(spraybus-client PROPERTIES EXPORT_NAME client)

if(SPRAYBUS_BUILD_SERVER)
  add_library(spraybus-server src/spraybus/server/server.cpp
                              src/spraybus/server/logging.cpp)
  add_library(spraybus::server ALIAS spraybus-server)
  spraybus_configure_library(spraybus-server)
  target_link_libraries(spraybus-server PUBLIC spraybus-networking quill::quill)
  set_target_properties(spraybus-server PROPERTIES EXPORT_NAME server)
endif()

if(SPRAYBUS_BUILD_APPS)
  add_executable(cli drivers/cli.cpp)
  target_link_libraries(cli PRIVATE spraybus-client)

  if(SPRAYBUS_BUILD_SERVER)
    add_executable(main drivers/main.cpp)
    target_link_libraries(main PRIVATE spraybus-server)
  endif()
endif()

if(SPRAYBUS_BUILD_PYTHON)
  add_subdirectory(python)
endif()

if(SPRAYBUS_INSTALL_CLIENT)
  set(spraybus_install_targets spraybus-networking spraybus-client)
  if(SPRAYBUS_BUILD_SERVER)
    list(APPEND spraybus_install_targets spraybus-server)
  endif()

  install(
    TARGETS ${spraybus_install_targets}
    EXPORT spraybus-clientTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(DIRECTORY include/spraybus/client
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus)
  install(DIRECTORY include/spraybus/networking/client
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus/networking)
  install(FILES include/spraybus/networking/networking.hpp
                include/spraybus/networking/protocol.hpp
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus/networking)

  if(SPRAYBUS_BUILD_SERVER)
    install(DIRECTORY include/spraybus/common
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus)
    install(DIRECTORY include/spraybus/server
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus)
    install(DIRECTORY include/spraybus/networking/server
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spraybus/networking)
  endif()

  if(SPRAYBUS_BUILD_APPS)
    install(TARGETS cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    if(SPRAYBUS_BUILD_SERVER)
      install(TARGETS main RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  endif()

  install(
    EXPORT spraybus-clientTargets
    FILE spraybus-clientTargets.cmake
    NAMESPACE spraybus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spraybus-client)

  configure_package_config_file(
    cmake/spraybus-clientConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/spraybus-clientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spraybus-client)

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/spraybus-clientConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/spraybus-clientConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/spraybus-clientConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spraybus-client)
endif()

find_package(
  Python
  COMPONENTS Interpreter Development
  REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_native src/spraybus/_bindings.cpp)
target_link_libraries(_native PRIVATE spraybus-client)
target_compile_features(_native PRIVATE cxx_std_23)
set_target_properties(_native PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                         "${CMAKE_CURRENT_BINARY_DIR}/spraybus")

install(
  DIRECTORY src/spraybus
  DESTINATION .
  FILES_MATCHING
  PATTERN "*.py"
  PATTERN "py.typed")
install(TARGETS _native LIBRARY DESTINATION spraybus)

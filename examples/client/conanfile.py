from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class SprayBusClientExampleConan(ConanFile):
    name = "spraybus-client-example"
    version = "0.1"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("spraybus-client/0.1")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

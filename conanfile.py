from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class SprayBusConan(ConanFile):
    name = "spraybus"
    version = "0.1"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False]}
    default_options = {"shared": False}

    generators = ("CMakeDeps", "CMakeToolchain")

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("quill/11.0.2")
        self.requires("boost/1.90.0")
        self.requires("enet/1.3.18")
        self.requires("prometheus-cpp/1.3.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        # cmake.test()

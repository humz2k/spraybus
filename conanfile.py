from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy

import os


class SprayBusClientConan(ConanFile):
    name = "spraybus-client"
    version = "0.1"
    package_type = "library"
    license = "Unlicense"
    url = "https://github.com/hqureshi/spraybus"
    description = "C++23 ENet-backed spraybus client library"
    topics = ("pubsub", "messaging", "enet")

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_server": [True, False],
        "with_apps": [True, False],
        "with_python": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_server": True,
        "with_apps": True,
        "with_python": True,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
        "drivers/*",
        "python/CMakeLists.txt",
        "python/src/spraybus/_bindings.cpp",
        "python/src/spraybus/*.py",
        "python/src/spraybus/py.typed",
        "LICENSE",
    )
    generators = "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def validate(self):
        check_min_cppstd(self, "23")

    def requirements(self):
        self.requires(
            "enet/1.3.18", transitive_headers=True, transitive_libs=True
        )
        if self.options.with_server:
            self.requires("quill/11.0.2")
        if self.options.with_python:
            self.requires("pybind11/2.13.6", libs=False, visible=False)

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["SPRAYBUS_BUILD_SERVER"] = bool(
            self.options.with_server
        )
        toolchain.variables["SPRAYBUS_BUILD_APPS"] = bool(self.options.with_apps)
        toolchain.variables["SPRAYBUS_BUILD_PYTHON"] = bool(
            self.options.with_python
        )
        toolchain.variables["SPRAYBUS_INSTALL_CLIENT"] = True
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(
            self,
            "LICENSE",
            self.source_folder,
            os.path.join(self.package_folder, "licenses"),
        )
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "spraybus-client")
        self.cpp_info.set_property("pkg_config_name", "spraybus-client")

        self.cpp_info.components["networking"].set_property(
            "cmake_target_name", "spraybus::networking"
        )
        self.cpp_info.components["networking"].libs = ["spraybus-networking"]
        self.cpp_info.components["networking"].requires = ["enet::enet"]

        self.cpp_info.components["client"].set_property(
            "cmake_target_name", "spraybus::client"
        )
        self.cpp_info.components["client"].libs = ["spraybus-client"]
        self.cpp_info.components["client"].requires = ["networking", "enet::enet"]

        if self.options.with_server:
            self.cpp_info.components["server"].set_property(
                "cmake_target_name", "spraybus::server"
            )
            self.cpp_info.components["server"].libs = ["spraybus-server"]
            self.cpp_info.components["server"].requires = [
                "networking",
                "quill::quill",
            ]

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy

class GTProxy(ConanFile):
    name = "GTProxy"
    author = "ZTz"
    settings = "os", "compiler", "build_type", "arch"
    requires = [
        "cpp-httplib/0.11.2",
        "fmt/8.1.1",
        "libressl/3.5.3",
        "magic_enum/0.8.2",
        "nlohmann_json/3.11.2",
        "pcg-cpp/cci.20210406",
        "spdlog/1.10.0",
    ]
    default_options = {
        "libressl*:shared": True
    }

    def layout(self):
        cmake_layout(self)

    def generate(self):
        for dep in self.dependencies.values():
            if dep.cpp_info.bindirs:
                copy(self, "*.dll", dep.cpp_info.bindirs[0], self.build_folder)
            if dep.cpp_info.libdirs:
                copy(self, "*.lib", dep.cpp_info.libdirs[0], self.build_folder)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

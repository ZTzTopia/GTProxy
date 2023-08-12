from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy

class GTProxyRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "libressl*:shared": True
    }

    def requirements(self):
        self.requires("cpp-httplib/[~0.12]")
        self.requires("fmt/8.1.1")
        self.requires("libressl/[~3.5]")
        self.requires("magic_enum/[~0.8]")
        self.requires("nlohmann_json/[~3.11]")
        self.requires("pcg-cpp/cci.20210406")
        self.requires("spdlog/1.10.0")

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

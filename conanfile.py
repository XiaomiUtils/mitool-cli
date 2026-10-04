from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class MitoolCliConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("libiconv/1.18", override=True)
        self.requires("openssl/3.6.2")
        self.requires("nlohmann_json/3.12.0")
        self.requires("libarchive/3.8.7")
        self.requires("cli11/2.7.2")
        self.requires("libcurl/8.20.0")
        self.requires("re2/20251105")

    def generate(self):
        CMakeDeps(self).generate()
        CMakeToolchain(self).generate()

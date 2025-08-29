from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os


class AgentixxConan(ConanFile):
    name = "agentixx"
    version = "0.1.0"
    
    # Metadata
    description = "Modern C++ library for AI agent interactions with LLM APIs"
    author = "Agentixx Team"
    topics = ("cpp", "ai", "llm", "openai", "api", "agents")
    url = "https://github.com/your-repo/agentixx"  # Update with actual repo URL
    license = "MIT"  # Update with actual license
    
    # Package configuration  
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_examples": [True, False],
        "build_tests": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_examples": False,
        "build_tests": False
    }
    
    # Sources are located in the parent directory
    exports_sources = "../CMakeLists.txt", "../src/*", "../include/*", "../examples/*", "../cmake/*", "../LICENSE"
    
    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")
    
    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
            
        # Remove runtime from settings if building a header-only parts
        if self.settings.compiler.cppstd:
            # Ensure C++20
            if str(self.settings.compiler.cppstd) < "20":
                self.output.warn(f"Agentixx requires C++20, but {self.settings.compiler.cppstd} was specified")
    
    def requirements(self):
        # Core dependencies
        self.requires("libcurl/[^8.0]")
        self.requires("nlohmann_json/[^3.11]")
        
        # Optional dependencies for examples/tests
        if self.options.build_tests:
            self.test_requires("gtest/[^1.14]")
    
    def build_requirements(self):
        self.tool_requires("cmake/[^3.24]")
    
    def layout(self):
        cmake_layout(self, src_folder="..")
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        
        # Pass options to CMake
        tc.variables["BUILD_EXAMPLES"] = self.options.build_examples  
        tc.variables["BUILD_TESTS"] = self.options.build_tests
        tc.variables["CMAKE_CXX_STANDARD"] = "20"
        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        
        # Release optimizations
        if self.settings.build_type == "Release":
            tc.variables["CMAKE_CXX_FLAGS_RELEASE"] = "-O3 -DNDEBUG"
            
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
        # Run tests if enabled
        if self.options.build_tests:
            cmake.test()
    
    def package(self):
        cmake = CMake(self)
        cmake.install()
        
        # Copy license
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
    
    def package_info(self):
        # Main library
        self.cpp_info.libs = ["agentixx"]
        
        # Include directories
        self.cpp_info.includedirs = ["include"]
        
        # C++20 requirement
        self.cpp_info.cppstd = "20"
        
        # System libs (if needed)
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["pthread"])
        
        # Compiler flags
        if self.settings.compiler == "gcc" or self.settings.compiler == "clang":
            self.cpp_info.cppflags.extend(["-Wall", "-Wextra"])
            
        # Build type specific flags  
        if self.settings.build_type == "Debug":
            self.cpp_info.defines.append("AGENTCPP_DEBUG")
        
        # Package components
        self.cpp_info.components["core"].libs = ["agentixx"]
        self.cpp_info.components["core"].requires = ["libcurl::libcurl", "nlohmann_json::nlohmann_json"]
        self.cpp_info.components["core"].includedirs = ["include"]
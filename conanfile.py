import os
import re

from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conans.tools import load
from conans import tools as ctools

def get_version():
    try:
        version = os.getenv('PROJECT_VERSION', None)
        if version:
            return version

        content = load('CMakeLists.txt')
        version = re.search('set\(PROJECT_VERSION (.*)\)', content).group(1)
        return version.strip()
    except:
        return None

class JsonSchemaValidatorConan(ConanFile):
    name = 'JsonSchemaValidator'
    version = get_version()
    url = 'https://github.com/pboettch/json-schema-validator'
    license = 'MIT'

    settings = 'os', 'compiler', 'build_type', 'arch'

    options = {
        'shared': [True, False],
        'fPIC': [True, False],
        'build_examples': [True, False],
        'build_tests': [True, False],
        'test_coverage': [True, False],
    }

    default_options = {
        'shared': False,
        'fPIC': True,
        'build_examples': True,
        'build_tests': False,
        'test_coverage': False,
    }

    generators = 'CMakeDeps', 'CMakeToolchain', 'VirtualBuildEnv', 'VirtualRunEnv'

    exports_sources = [
        'CMakeLists.txt',
        'conanfile.py',
        'cmake/*',
        'src/*',
        'example/*',
        'test/*',
    ]

    requires = [
        'nlohmann_json/3.11.2'
    ]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables['JSON_VALIDATOR_BUILD_EXAMPLES'] = self.options.build_examples
        tc.variables['JSON_VALIDATOR_BUILD_TESTS'] = self.options.build_tests
        tc.variables['JSON_VALIDATOR_SHARED_LIBS '] = self.options.shared
        tc.variables['JSON_VALIDATOR_TEST_COVERAGE '] = self.options.test_coverage
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.verbose = True
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        includedir = os.path.join(self.package_folder, "include")
        self.cpp_info.includedirs = [includedir]

        libdir = os.path.join(self.package_folder, "lib")
        self.cpp_info.libdirs = [libdir]
        self.cpp_info.libs += ctools.collect_libs(self, libdir)

        bindir = os.path.join(self.package_folder, "bin")
        self.output.info("Appending PATH environment variable: {}".format(bindir))
        self.env_info.PATH.append(bindir)

        self.user_info.VERSION = self.version

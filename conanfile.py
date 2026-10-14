import os
import re

from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeToolchain
from conan.tools.files import collect_libs, load


class JsonSchemaValidatorConan(ConanFile):
    name = 'json-schema-validator'
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

    implements = ['auto_shared_fpic']

    generators = 'CMakeDeps'

    exports_sources = [
        'CMakeLists.txt',
        'conanfile.py',
        'cmake/*',
        'src/*',
        'example/*',
        'test/*',
    ]

    def set_version(self):
        version = os.getenv('PROJECT_VERSION', None)
        if not version:
            content = load(self, os.path.join(self.recipe_folder, 'CMakeLists.txt'))
            version = re.search(r'set\(PROJECT_VERSION (.*)\)', content).group(1)
        self.version = version.strip()

    def requirements(self):
        # the public header includes nlohmann/json.hpp
        self.requires('nlohmann_json/3.11.2', transitive_headers=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables['JSON_VALIDATOR_BUILD_EXAMPLES'] = bool(self.options.build_examples)
        tc.cache_variables['JSON_VALIDATOR_BUILD_TESTS'] = bool(self.options.build_tests)
        tc.cache_variables['JSON_VALIDATOR_SHARED_LIBS'] = bool(self.options.shared)
        tc.cache_variables['JSON_VALIDATOR_TEST_COVERAGE'] = bool(self.options.test_coverage)
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property('cmake_file_name', 'nlohmann_json_schema_validator')
        self.cpp_info.set_property('cmake_target_name', 'nlohmann_json_schema_validator::validator')
        self.cpp_info.libs = collect_libs(self)
        if self.options.shared:
            self.cpp_info.defines = ['JSON_SCHEMA_VALIDATOR_IMPORTS']

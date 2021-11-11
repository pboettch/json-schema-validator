import os
import re
from conans import load, tools, ConanFile, CMake


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
        'fPIC': [True, False]
    }
    default_options = {
        'shared': False,
        'fPIC': True
    }
    generators = "cmake"
    exports_sources = [
        'CMakeLists.txt',
        'nlohmann_json_schema_validatorConfig.cmake.in',
        'src/*',
        'app/*',
    ]

    requires = (
        'nlohmann_json/3.7.3'
    )

    def build(self):
        cmake = CMake(self)
        cmake.definitions['nlohmann_json_DIR'] = os.path.join(self.deps_cpp_info['nlohmann_json'].rootpath, 'include')
        cmake.definitions['JSON_VALIDATOR_BUILD_EXAMPLES'] = True
        cmake.definitions['JSON_VALIDATOR_BUILD_TESTS'] = False
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        includedir = os.path.join(self.package_folder, "include")
        self.cpp_info.includedirs = [includedir]

        libdir = os.path.join(self.package_folder, "lib")
        self.cpp_info.libdirs = [libdir]
        self.cpp_info.libs += tools.collect_libs(self, libdir)

        bindir = os.path.join(self.package_folder, "bin")
        self.output.info("Appending PATH environment variable: {}".format(bindir))
        self.env_info.PATH.append(bindir)

        self.user_info.VERSION = self.version

# Modern C++ JSON schema validator

# What is it?

This is a C++ header-only library for validating JSON documents based on a
[JSON Schema](http://json-schema.org/) which itself should validate with
[draft-4 of JSON Schema Validation](http://json-schema.org/schema).

First a disclaimer: *Everything here should be considered work in progress and
contributions or hints or discussions are welcome.*

Niels Lohmann et al develop a great JSON parser for C++ called [JSON for Modern
C++](https://github.com/nlohmann/json). This validator is based on this
library, hence the name.

The name is for the moment purely marketing, because there is, IMHO, not much
modern C++ inside. But I think the whole thing could be rewritten mode "modern".

External documentation is missing as well. However the API of the validator
will be rather simple.

# How to use

## Build

Directly

```Bash
git clone https://github.com/pboettch/json-schema-validator.git
cd json-schema-validator
mkdir build
cd build
cmake .. \
    -DNLOHMANN_JSON_DIR=<path/to/json.hpp> \
    -DJSON_SCHEMA_TEST_SUITE_PATH=<path/to/JSON-Schema-test-suite> # optional
make # install
ctest # if test-suite has been given
```
or from another CMakeLists.txt as a subdirectory:

```CMake
# create an interface-target called json-hpp
add_library(json-hpp INTERFACE)
target_include_directories(json-hpp
    INTERFACE
        path/to/json.hpp)
set(JSON_SCHEMA_TEST_SUITE_PATH "path/to/json-schema-test-suite")

enable_testing() # if you want to inherit tests
add_subdirectory(path-to-this-project json-schema-validator)
```

## Code

See also `app/json-schema-validate.cpp`.

```C++
#include "json-schema-validator.hpp"

using nlohmann::json;
using nlohmann::json_validator;

int main(void)
{
	json schema, document;

    /* fill in the schema */
    /* fill in the document */

	json_validator validator;

    try {
        validator.validate(document, scheam);
    } catch (const std::out_of_range &e) {
        std::cerr << "Validation failed, here is why: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```

# Conformity

There is an application which can be used for testing the validator with the
[JSON-Schema-Test-Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite).

Currently **82** of ~**308** tests are still failing, because simply not all keywords and
their functionalities have been implemented. Some of the missing feature will
require a rework. Some will only work with external libraries. (remote references)

# Additional features

## Default value population

For my use case I need something to populate default values into the JSON
instance of properties which are not set by the user.

This feature can be enable by setting the `default_value_insertion` to true.

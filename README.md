
[![Build Status](https://travis-ci.org/pboettch/json-schema-validator.svg?branch=master)](https://travis-ci.org/pboettch/json-schema-validator)

# JSON schema validator for JSON for Modern C++

# What is it?

This is a C++ library for validating JSON documents based on a
[JSON Schema](http://json-schema.org/) which itself should validate with
[draft-7 of JSON Schema Validation](http://json-schema.org/schema).

First a disclaimer: *It is work in progress and
contributions or hints or discussions are welcome.* Even though a 2.0.0 release is imminent.

Niels Lohmann et al develop a great JSON parser for C++ called [JSON for Modern
C++](https://github.com/nlohmann/json). This validator is based on this
library, hence the name.

External documentation is missing as well. However the API of the validator
is rather simple.

# New in version 2

Although significant changes have been done for the 2nd version
(a complete rewrite) the API is compatible with the 1.0.0 release. Except for
the namespace which is now `nlohmann::json_schema.

Version **2** supports JSON schema draft 7, whereas 1 was supporting draft 4
only. Please update your schemas.

The primary change in 2 is the way a schema is used. While in version 1 the schema was
kept as a JSON-document and used again and again during validation, in version 2 the schema
is parsed into compiled C++ objects which are then used during validation. There are surely
still optimizations to be done, but validation speed has improved by factor 100
or more.

# Design goals

The main goal of this validator is to produce *human-comprehensible* error
messages if a JSON-document/instance does not comply to its schema.

By default this is done with exceptions thrown at the users with a helpful
message telling what's wrong with the document while validating.

Starting with **2.0.0** the user can pass a `json_schema::basic_error_handler`-derived
object along with the instance to validate to receive a callback each time
a validation error occurs and decide what to do (throwing, counting, collecting).

Another goal was to use Niels Lohmann's JSON-library. This is why the validator
lives in his namespace.

# Thread-safety

Instance validation is thread-safe and the same validator-object can be used by
different threads:

The validate method is `const` which indicates the object is not modified when
being called:

```C++
	json json_validator::validate(const json &) const;
```

Validator-object creation however is not thread-safe. A validator has to be
created in one (main?) thread once.

# Weaknesses

Numerical validation uses nlohmann-json's integer, unsigned and floating point
types, depending on if the schema type is "integer" or "number". Bignum
(i.e. arbitrary precision and range) is not supported at this time.

# Building

This library is based on Niels Lohmann's JSON-library and thus has
a build-dependency to it.

Currently at least version **3.6.0** of NLohmann's JSON library
is required.

Various methods using CMake can be used to build this project.

## Build out-of-source

Do not run cmake inside the source-dir. Rather create a dedicated build-dir:

```Bash
git clone https://github.com/pboettch/json-schema-validator.git
cd json-schema-validator
mkdir build
cd build
cmake [..]
make
make install # if needed
ctest # run unit, non-regression and test-suite tests
```

## Building as shared library

By default a static library is built. Shared libraries can be generated by using
the `BUILD_SHARED_LIBS`-cmake variable:

In your initial call to cmake simply add:
```bash
cmake [..] -DBUILD_SHARED_LIBS=ON [..]
```

## nlohmann-json integration

As nlohmann-json is a dependency, this library tries find it.

The cmake-configuration first checks if nlohmann-json is available as an cmake-target. This may happen, because it is used as a submodule in a super-project which already provides and uses nlohmann-json.
Otherwise, it calls `find_package` for nlohmann-json and requires that library to be installed on the system.

### Building with Hunter package manager

To enable access to nlohmann json library, Hunter can be used. Just run with HUNTER_ENABLED=ON option. No further dependencies needed

```bash
cmake [..] -DHUNTER_ENABLED=ON [..]
```

### Building as a CMake-subdirectory from within another project

Adding this library as a subdirectory to a parent project is one way of
building it.

If the parent project

- already used `find_package()` to find the CMake-package of nlohmann_json, method 1 will work.
- uses the git-repo of nlohmann_json as a subdirectory, method 1 will work.
- sets nlohmann_json_DIR, method 2 or 3 will work.

Afterwards a target called `nlohmann_json_schema_validator`
is available in order to compile and link.

### Building directly, finding a CMake-package. (short)

When nlohmann-json has been installed, it provides files which allows
CMake's `find_package()` to be used.

This library is using this mechanism if `nlohmann_json::nlohmann_json`-target
does not exist.

The variable `nlohmann_json_DIR` can be used to help `find_package()` find this package.

### Building directly: provide a path to where to find json.hpp

The last method before fataling out is by providing a path where the file json.hpp can be found.

The variable `nlohmann_json_DIR` has to be used to point to the path
where `json.hpp` is found in a subdirectory called `nlohmann`, e.g.:

`json.hpp` is located in `/path/to/nlohmann/json.hpp`. The `cmake`-command has to be run as follows:

```bash
cmake -Dnlohmann_json_DIR=/path/to [..]
```

### Method 1 - long version

Since version 2.1.0 this library can be installed and CMake-package-files will be
created accordingly. If the installation of nlohmann-json and this library
is done into default unix-system-paths CMake will be able to find this
library by simply doing:

```CMake
find_package(nlohmann_json REQUIRED)
find_package(nlohmann_json_schema_validator REQUIRED)
```

and

```CMake
target_link_libraries(<your-target> [..] nlohmann_json_schema_validator)
```
to build and link.

If a custom path has been used to install this library (and nlohmann-json), `find_package()`
needs a hint for where to find the package-files, it can be provided by setting the following variables

```CMake
cmake .. \
    -Dnlohmann_json_DIR=<path/to/>lib/cmake/nlohmann_json \
	-Dnlohmann_json_schema_validator_DIR:PATH=<path/to/>/lib/cmake/nlohmann_json_schema_validator
```

Note that if the this library is used as cmake-package, nlohmann-json also has
to be used a cmake-package.

## Code

See also `app/json-schema-validate.cpp`.

```C++
#include <iostream>
#include <iomanip>

#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

// The schema is defined based upon a string literal
static json person_schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "A person",
    "properties": {
        "name": {
            "description": "Name",
            "type": "string"
        },
        "age": {
            "description": "Age of the person",
            "type": "number",
            "minimum": 2,
            "maximum": 200
        }
    },
    "required": [
                 "name",
                 "age"
                 ],
    "type": "object"
}

)"_json;

// The people are defined with brace initialization
static json bad_person = {{"age", 42}};
static json good_person = {{"name", "Albert"}, {"age", 42}};

int main()
{
    /* json-parse the schema */

    json_validator validator; // create validator

    try {
        validator.set_root_schema(person_schema); // insert root-schema
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema failed, here is why: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    /* json-parse the people - API of 1.0.0, default throwing error handler */

    for (auto &person : {bad_person, good_person}) {
        std::cout << "About to validate this person:\n"
                  << std::setw(2) << person << std::endl;
        try {
            validator.validate(person); // validate the document - uses the default throwing error-handler
            std::cout << "Validation succeeded\n";
        } catch (const std::exception &e) {
            std::cerr << "Validation failed, here is why: " << e.what() << "\n";
        }
    }

    /* json-parse the people - with custom error handler */
    class custom_error_handler : public nlohmann::json_schema::basic_error_handler
    {
        void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer, const json &instance,
            const std::string &message) override
        {
            nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
            std::cerr << "ERROR: '" << pointer << "' - '" << instance << "': " << message << "\n";
        }
    };


    for (auto &person : {bad_person, good_person}) {
        std::cout << "About to validate this person:\n"
                  << std::setw(2) << person << std::endl;

        custom_error_handler err;
        validator.validate(person, err); // validate the document

        if (err)
            std::cerr << "Validation failed\n";
        else
            std::cout << "Validation succeeded\n";
    }

    return EXIT_SUCCESS;
}
```

# Compliance

There is an application which can be used for testing the validator with the
[JSON-Schema-Test-Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite).
In order to simplify the testing, the test-suite is included in the repository.

If you have cloned this repository providing a path the repository-root via the
cmake-variable `JSON_SCHEMA_TEST_SUITE_PATH` will enable the test-target(s).

All required tests are **OK**.

# Format

Optionally JSON-schema-validator can validate predefined or user-defined formats.
Therefore a format-checker-function can be provided by the user which is called by
the validator when a format-check is required (ie. the schema contains a format-field).

This is how the prototype looks like and how it can be passed to the validation-instance:

```C++
static void my_format_checker(const std::string &format, const std::string &value)
{
	if (format == "something") {
		if (!check_value_for_something(value))
			throw std::invalid_argument("value is not a good something");
	} else
		throw std::logic_error("Don't know how to validate " + format);
}

// when creating the validator

json_validator validator(nullptr, // or loader-callback
                         my_format_checker); // create validator
```

The library contains a default-checker, which does some checks. It needs to be
provided manually to the constructor of the validator:

```C++
json_validator validator(loader, // or nullptr for no loader
                         nlohmann::json_schema::default_string_format_check);
```

# Contributing

Before opening a pull request, please apply the coding style given in the
`.clang-format` by running clang-format from the git top-level for all touched
files:

```shell
git diff master --name-only | grep '\.[ch]pp$' | xargs -P 3 -I{} clang-format -i {}
```

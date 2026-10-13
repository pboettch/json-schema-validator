[![Tests](https://github.com/pboettch/json-schema-validator/actions/workflows/test.yaml/badge.svg)](https://github.com/pboettch/json-schema-validator/actions/workflows/test.yaml)

# JSON schema validator for JSON for Modern C++

# What is it?

This is a C++ library for validating JSON documents based on a
[JSON Schema](http://json-schema.org/) which itself should validate with
[draft-7 of JSON Schema Validation](http://json-schema.org/schema).

First a disclaimer: *It is work in progress and
contributions or hints or discussions are welcome.*

Niels Lohmann et al develop a great JSON parser for C++ called [JSON for Modern
C++](https://github.com/nlohmann/json). This validator is based on this
library, hence the name.

External documentation is missing as well. However the API of the validator
is rather simple.

# New in version 2

Although significant changes have been done for the 2nd version
(a complete rewrite) the API is compatible with the 1.0.0 release. Except for
the namespace which is now `nlohmann::json_schema`.

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

Currently at least version **3.8.0** of NLohmann's JSON library
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

By default a static library is built. A shared library is built with the option `JSON_VALIDATOR_SHARED_LIBS`,
whose default is the value of CMake's `BUILD_SHARED_LIBS`:

```bash
cmake [..] -DJSON_VALIDATOR_SHARED_LIBS=ON [..]
```

On Windows, linking with the CMake target `nlohmann_json_schema_validator::validator` defines
`JSON_SCHEMA_VALIDATOR_IMPORTS`, which is needed to use the DLL.

## nlohmann-json integration

As nlohmann-json is a dependency, the CMake configuration looks for it in this order:

1. If a CMake target `nlohmann_json` already exists, it is used. This is the case when a parent project added
   nlohmann-json with `add_subdirectory()` or `FetchContent` before adding this library.
1. Otherwise an installed nlohmann-json is searched with `find_package()`.
1. If none is found, nlohmann-json is fetched from GitHub with `FetchContent`. The version is set with
   `JSON_FETCH_VERSION` (default `v3.12.0`):

```bash
cmake [..] -DJSON_FETCH_VERSION=v3.11.3 [..]
```

This library and the code using it must be compiled with the same version of nlohmann-json: since version 3.11 its
symbols are versioned and a mismatch results in link errors.

### Building as a CMake-subdirectory from within another project

This library can be added to a parent project with `add_subdirectory()` or `FetchContent`. Its tests, examples and
install rules are then disabled by default. Link your target with `nlohmann_json_schema_validator::validator`.

### Install

Since version 2.1.0 this library can be installed and CMake-package-files will be
created accordingly. If the installation of nlohmann-json and this library
is done into default unix-system-paths CMake will be able to find this
library by simply doing:

```CMake
find_package(nlohmann_json_schema_validator REQUIRED)
```

and

```CMake
target_link_libraries(<your-target> [..] nlohmann_json_schema_validator::validator)
```

to build and link.

Before version 2.3.0 the target was named `nlohmann_json_schema_validator` (without namespace).

### Installing when used via FetchContent

When this library is included in another project with `FetchContent` (or `add_subdirectory()`), its install
rules are disabled by default. Do not add `nlohmann_json_schema_validator` to your own `install(TARGETS ...)`:
the header will not be found or will be installed to the wrong location. Instead, enable the library's own
install rules before making it available:

```CMake
include(FetchContent)

set(JSON_VALIDATOR_INSTALL ON)
FetchContent_Declare(nlohmann_json_schema_validator
    GIT_REPOSITORY https://github.com/pboettch/json-schema-validator
    GIT_TAG        2.4.1)
FetchContent_MakeAvailable(nlohmann_json_schema_validator)

target_link_libraries(<your-target> PRIVATE nlohmann_json_schema_validator::validator)
```

If nlohmann-json is fetched as well, it will be installed alongside (`JSON_Install` is enabled automatically).

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

## Default Checker

The library contains a default-checker, which does some checks. It needs to be
provided manually to the constructor of the validator:

```C++
json_validator validator(loader, // or nullptr for no loader
                         nlohmann::json_schema::default_string_format_check);
```

Supported formats: `date-time, date, time, email, idn-email, hostname, ipv4, ipv6, uri, uri-reference, iri, iri-reference, uri-template, json-pointer, relative-json-pointer, uuid, regex`

More formats can be added in `src/string-format-check.cpp`. Please contribute implementions for missing json schema draft formats.

## Default value processing

As a result of the validation, the library returns a json patch including the default values of the specified schema.

```C++
#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static const json rectangle_schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "A rectangle",
    "properties": {
        "width": {
            "$ref": "#/definitions/length",
            "default": 20
        },
        "height": {
            "$ref": "#/definitions/length"
        }
    },
    "definitions": {
        "length": {
            "type": "integer",
            "minimum": 1,
            "default": 10
        }
    }
})"_json;

int main()
{
	try {
		json_validator validator{rectangle_schema};
		/* validate empty json -> will be expanded by the default values defined in the schema */
		json rectangle = "{}"_json;
		const auto default_patch = validator.validate(rectangle);
		rectangle = rectangle.patch(default_patch);
		std::cout << rectangle.dump() << std::endl; // {"height":10,"width":20}
	} catch (const std::exception &e) {
		std::cerr << "Validation of schema failed: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
```

The example above will output the specified default values `{"height":10,"width":20}` to stdout.

> Note that the default value specified in a `$ref` may be overridden by the current instance location. Also note that this behavior will break draft-7, but it is compliant to newer drafts (e.g. `2019-09` or `2020-12`).

Default values are added one level at a time:

- The `default` of a property is added when the property is missing. A property which is present is kept, even if its
  value is `null`.
- The `default` of an object is used when the object itself is missing. It does not fill in properties of an object
  which is present.
- `"default": null` is treated as no default value.
- An empty document (`null`) is replaced by the `default` of the root schema.

The defaults inside an added default value are only found by the next validation. To get all nested default
values, validate and apply the patch until it is empty:

```C++
json instance = "{}"_json;
for (auto patch = validator.validate(instance); !patch.empty(); patch = validator.validate(instance))
	instance = instance.patch(patch);
```

# Contributing

This project uses [`pre-commit`](https://pre-commit.com/) to enforce style-checks. Please install and run it before
creating commits and making pull requests.

```console
$ pip install pre-commit
$ pre-commit install
```

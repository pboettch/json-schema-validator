#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static const json default_schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "oneOf": [
        {
            "type": "object",
            "properties": {
                "name": {
                    "enum": "foo"
                },
                "code": {
                    "const": 1,
                    "default": 1
                }
            }
        },
        {
            "type": "object",
            "properties": {
                "name": {
                    "enum": "bar"
                },
                "code": {
                    "const": 2,
                    "default": 2
                }
            }
        }
    ]
})"_json;

static void loader(const json_uri &uri, json &schema)
{
	schema = default_schema;
}

int main(void)
{
	json_validator validator(loader);

	validator.set_root_schema(default_schema);

	json data = R"({"name": "bar"})"_json;
	json expected = R"(
        [
            {
                "op": "add",
                "path": "/code",
                "value": 2
            }
        ]
    )"_json;

	json patch = validator.validate(data);
	if (patch != expected) {
		std::cerr << "Patch contains wrong operation: '" << patch.dump() << "' instead of expected '" << expected.dump() << "'" << std::endl;
		return 1;
	}

	return 0;
}

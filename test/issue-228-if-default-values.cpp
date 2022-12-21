#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static const json default_schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "condition": {
            "type": "boolean",
            "default": false
        },
        "data": {
            "type": "string",
            "default": "default"
        }
    },
    "if": {
        "properties": { "condition": { "const": true } }
    },
    "then": {
        "required": ["data"]
    }
})"_json;

static void loader(const json_uri &uri, json &schema)
{
	schema = default_schema;
}

int main(void)
{
	json_validator validator(loader);

	validator.set_root_schema(default_schema);

	json empty_object = json::object();
	validator.validate(empty_object);

	json enable_condition = R"({"condition": true})"_json;
	auto patch = validator.validate(enable_condition);
	enable_condition.patch_inplace(patch);
	if (enable_condition.at("data") != "default") {
		std::cerr << "Unexpected data: " << enable_condition.at("data") << " instead of 'default'" << std::endl;
		return 1;
	}

	return 0;
}

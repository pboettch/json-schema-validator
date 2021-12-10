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
			"default": 10
    	}
	},
    "additionalProperties": false,
    "type": "object"
})"_json;

int main(void)
{
	json_validator validator{};
	validator.set_root_schema(rectangle_schema);

	{
		json empty_rectangle = R"({})"_json;

		const auto default_patch = validator.validate(empty_rectangle);
		const auto actual = empty_rectangle.patch(default_patch);

		// height must be 10 according to the default specified in the length definition while width must be 10 overridden by the width element
		const auto expected = R"({"height":10,"width":20})"_json;
		if (actual != expected) {
			std::cerr << "Patch with defaults contains wrong value: '" << actual << "' instead of expected '" << expected.dump() << "'" << std::endl;
			return 1;
		}
	}

	return 0;
}

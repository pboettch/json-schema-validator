#include "nlohmann/json-schema.hpp"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

/*
 * This schema describes a JSON object with two mutually exclusive
 * properties "a" and "b" enforced through a "not" - "required"
 * pattern. The "not"-schema matches only if both of the mutually
 * exclusive properties are present through its "required".
 *
 * This schema includes an uncommon edge-case: There are default
 * values specified in the properties of the "not"-schema as well as
 * in the dummy "if"-schema. These should only serve as conditions
 * for the evaluation result or the evaluation of "then"/"else"
 * subschemas, but shouldn't contribute default values to the final
 * default-value patch themselves.
 */
static auto schema = json::parse(R"JSON({
	"type": "object",

	"properties": {
		"a": {
			"type": "boolean"
		},
		"b": {
			"type": "boolean"
		}
	},

	"if": {
		"required": ["a", "b"],
		"properties": {
			"a": {
				"type": "boolean",
				"default": true
			}
		}
	},

	"not": {
		"required": ["a", "b"],
		"properties": {
			"b": {
				"type": "boolean",
				"default": true
			}
		}
	}
})JSON");

int main(void)
{
	auto validator = json_validator(schema);
	auto example = json::object();
	auto default_patch = validator.validate(example);

	if (not default_patch.empty()) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

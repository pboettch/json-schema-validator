#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

// Test: required property with default should be filled, not error
static const json schema = R"(
{
    "properties": {
        "x": {"type": "string", "default": "foo"}
    },
    "required": ["x"]
})"_json;

int main(void)
{
	json_validator validator{};
	validator.set_root_schema(schema);

	json instance = R"({})"_json;

	// Use throwing error handler - validation must succeed
	nlohmann::json_schema::basic_error_handler err;
	const auto patch = validator.validate(instance, err);
	if (err)
		return 1;

	const auto result = instance.patch(patch);

	if (result["x"] != "foo")
		return 1;

	return 0;
}

#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

// Test: object with additionalProperties containing null value should validate
// additionalProperties causes traversal into the object where null value triggers the bug
static const json schema = R"(
{
    "properties": {
        "x": {"type": "object", "additionalProperties": {}}
    }
})"_json;

int main(void)
{
	json_validator validator{};
	validator.set_root_schema(schema);

	json instance = R"({"x": {"nested": null}})"_json;

	const auto patch = validator.validate(instance);
	const auto result = instance.patch(patch);

	// null value should be preserved
	if (!result["x"]["nested"].is_null()) {
		std::cerr << "Failed: nested should be null, got: " << result["x"]["nested"].dump() << std::endl;
		return 1;
	}

	return 0;
}

#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

// null values inside a document are values, they must not generate a default-patch
// (which used to be an add-operation on the root, replacing the whole document, issue-380)
static int check(const json &schema, const json &instance)
{
	json_validator validator{};
	validator.set_root_schema(schema);

	const auto patch = validator.validate(instance);
	const auto result = instance.patch(patch);

	if (!patch.empty() || result != instance) {
		std::cerr << "Failed: schema " << schema.dump() << ", instance " << instance.dump()
		          << ": patch " << patch.dump() << " gives " << result.dump() << std::endl;
		return 1;
	}

	return 0;
}

int main(void)
{
	int failed = 0;

	// additionalProperties causes traversal into the object where the null value triggered the bug
	failed += check(R"({"properties": {"x": {"type": "object", "additionalProperties": {}}}})"_json,
	                R"({"x": {"nested": null}})"_json);

	// a null default for a present null value
	failed += check(R"({"properties": {"test": {"default": null}}})"_json,
	                R"({"test": null})"_json);

	// a present null value is not replaced by its default
	failed += check(R"({"properties": {"x": {"default": {"a": 1}}}})"_json,
	                R"({"x": null})"_json);

	// a root-default is only applied to an empty document, not to a document containing null values
	failed += check(R"({"properties": {"a": {"type": "null"}}, "default": {"a": null}})"_json,
	                R"({"a": null})"_json);

	return failed;
}

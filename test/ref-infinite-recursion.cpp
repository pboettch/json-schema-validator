#include <nlohmann/json-schema.hpp>

#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static int failures = 0;

class message_error_handler : public nlohmann::json_schema::basic_error_handler
{
public:
	std::string messages;

	void error(const json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
		basic_error_handler::error(ptr, instance, message);
		messages += message + "\n";
	}
};

// the schema is followed in a loop without consuming a part of the instance, validation has to stop
// with an error instead of overflowing the stack
static void expect_recursion_error(const json &schema, const json &instance)
{
	json_validator validator(schema);
	message_error_handler err;
	validator.validate(instance, err);

	if (!err || err.messages.find("infinite recursion") == std::string::npos) {
		std::cerr << instance.dump() << " against " << schema.dump() << " should report an infinite recursion, got: " << err.messages << "\n";
		failures++;
	}
}

static void expect_valid(const json &schema, const json &instance, const json &patch = json::array())
{
	json_validator validator(schema);
	message_error_handler err;
	auto result = validator.validate(instance, err);

	if (err) {
		std::cerr << instance.dump() << " against " << schema.dump() << " should be valid, got: " << err.messages << "\n";
		failures++;
	}
	if (result != patch) {
		std::cerr << instance.dump() << " against " << schema.dump() << " should give the patch " << patch.dump() << ", got " << result.dump() << "\n";
		failures++;
	}
}

int main(void)
{
	expect_recursion_error(json::parse(R"({"$ref": "#"})"), json::object());
	expect_recursion_error(json::parse(R"({"allOf": [{"$ref": "#"}]})"), 1);
	expect_recursion_error(json::parse(R"({"definitions": {"a": {"$ref": "#/definitions/b"}, "b": {"$ref": "#/definitions/a"}}, "$ref": "#/definitions/a"})"), 1);

	// the recursion is below the root
	expect_recursion_error(json::parse(R"({"definitions": {"a": {"anyOf": [{"$ref": "#/definitions/a"}]}}, "properties": {"x": {"$ref": "#/definitions/a"}}})"),
	                       json::parse(R"({"x": [1, 2]})"));

	// a cycle of references without a default has no default
	expect_valid(json::parse(R"({"definitions": {"a": {"$ref": "#/definitions/a"}}, "properties": {"x": {"$ref": "#/definitions/a"}}})"), json::object());

	// the same reference for a different instance is not a recursion
	const json tree = json::parse(R"({
		"definitions": {
			"node": {
				"type": "object",
				"properties": {
					"left": {"$ref": "#/definitions/node"},
					"right": {"$ref": "#/definitions/node"}
				}
			}
		},
		"$ref": "#/definitions/node"
	})");
	expect_valid(tree, json::parse(R"({"left": {"left": {}, "right": {}}, "right": {"right": {"left": {}}}})"));

	// the default value of a reference which is being followed
	const json list = json::parse(R"({
		"definitions": {
			"item": {
				"type": "object",
				"properties": {"next": {"$ref": "#/definitions/item"}},
				"default": {}
			}
		},
		"$ref": "#/definitions/item"
	})");
	expect_valid(list, json::object(), json::parse(R"([{"op": "add", "path": "/next", "value": {}}])"));

	return failures;
}

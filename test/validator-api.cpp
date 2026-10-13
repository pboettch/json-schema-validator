/*
 * JSON schema validator for JSON for modern C++
 *
 * SPDX-License-Identifier: MIT
 *
 */
// Errors when the json_validator is used wrongly or the schema cannot be resolved, and its move operations.
#include <nlohmann/json-schema.hpp>

#include <cstdlib>
#include <iostream>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static int errors;

// fun must throw an exception whose message contains the expected text
template <typename F>
static void expect_exception(const std::string &test, F fun, const std::string &expected)
{
	try {
		fun();
		std::cerr << "FAILED: " << test << ": no exception, expected '" << expected << "'\n";
		errors++;
	} catch (const std::exception &e) {
		if (std::string(e.what()).find(expected) == std::string::npos) {
			std::cerr << "FAILED: " << test << ": '" << e.what() << "' does not contain '" << expected << "'\n";
			errors++;
		}
	}
}

class collecting_error_handler : public nlohmann::json_schema::basic_error_handler
{
public:
	std::string messages;

	void error(const json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
		nlohmann::json_schema::basic_error_handler::error(ptr, instance, message);
		messages += message + "\n";
	}
};

static void expect_error(const std::string &test, const json_validator &validator, const json_uri &initial_uri, const std::string &expected)
{
	collecting_error_handler handler;
	validator.validate(1, handler, initial_uri);
	if (handler.messages.find(expected) == std::string::npos) {
		std::cerr << "FAILED: " << test << ": '" << handler.messages << "' does not contain '" << expected << "'\n";
		errors++;
	}
}

static void no_root_schema()
{
	json_validator validator;
	expect_exception(
	    "validate without root schema", [&]() { validator.validate(1); }, "no root schema has yet been set");
}

static void initial_uri()
{
	json_validator validator(json::parse(R"({"definitions": {"integer": {"type": "integer"}}})"));

	expect_error("initial URI of an unknown file", validator, json_uri("http://example.com/other.json"),
	             "no file found serving requested root-URI");
	expect_error("initial URI of an unknown subschema", validator, json_uri("#/definitions/missing"),
	             "for request initial URI");
}

static void no_loader()
{
	json_validator validator;
	expect_exception(
	    "external reference without loader",
	    [&]() { validator.set_root_schema(json::parse(R"({"properties": {"a": {"$ref": "http://example.com/other.json"}}})")); },
	    "needs loading, but no loader callback given");

	// the reference stays unresolved, validating reports it
	expect_exception(
	    "validate an unresolved reference", [&]() { validator.validate(json::parse(R"({"a": 1})")); },
	    "unresolved or freed schema-reference http://example.com/other.json");
	expect_exception(
	    "default value of an unresolved reference", [&]() { validator.validate(json::object()); },
	    "unresolved or freed schema-reference http://example.com/other.json");
}

static void unresolved_references()
{
	json_validator validator;
	expect_exception(
	    "unresolved references",
	    [&]() { validator.set_root_schema(json::parse(R"({"properties": {"a": {"$ref": "#/definitions/missing1"}, "b": {"$ref": "#/definitions/missing2"}}})")); },
	    "[/definitions/missing1, /definitions/missing2]");
}

static void duplicate_id()
{
	json_validator validator;
	expect_exception(
	    "duplicate $id",
	    [&]() { validator.set_root_schema(json::parse(R"({"definitions": {"a": {"$id": "http://example.com/a.json"}, "b": {"$id": "http://example.com/a.json"}}})")); },
	    "already inserted");
}

static void move()
{
	json_validator validator(json::parse(R"({"type": "integer"})"));

	json_validator moved(std::move(validator));
	moved.validate(1);
	expect_exception(
	    "move constructed", [&]() { moved.validate("one"); }, "unexpected instance type");

	json_validator assigned;
	assigned = std::move(moved);
	assigned.validate(1);
	expect_exception(
	    "move assigned", [&]() { assigned.validate("one"); }, "unexpected instance type");
}

int main()
{
	no_root_schema();
	initial_uri();
	no_loader();
	unresolved_references();
	duplicate_id();
	move();

	return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}

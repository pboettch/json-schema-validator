#include <nlohmann/json-schema.hpp>

#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

int main()
{
	json schema = json::parse(R"({"type": "object", "properties": {"answer": {"type": "integer"}}})");

	// the exported meta-schema validates the schema, it uses format
	json_validator meta(nlohmann::json_schema::draft7_schema_builtin, nullptr,
	                    nlohmann::json_schema::default_string_format_check);
	meta.validate(schema);

	json_validator validator(schema);
	validator.validate(json::parse(R"({"answer": 42})"));

	try {
		validator.validate(json::parse(R"({"answer": "42"})"));
	} catch (const std::exception &e) {
		std::cout << "invalid instance rejected: " << e.what() << "\n";
		return 0;
	}

	std::cerr << "invalid instance was accepted\n";
	return 1;
}

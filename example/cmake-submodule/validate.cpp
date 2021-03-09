#include <nlohmann/json-schema.hpp>

#include <iostream>

int main(void)
{
	nlohmann::json schema_json{{"type", "number"}};
	nlohmann::json_schema::json_validator validator;

	validator.set_root_schema(schema_json);

	validator.validate(1);
	try {
		validator.validate("\"1\"");
	} catch (const std::exception &e) {
		std::cerr << "expected exception: " << e.what() << "\n";
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

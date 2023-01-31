#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static const json read_only_schema = R"({
	"type": "object",
	"properties": {
		"debug": {
			"type": "boolean", 
			"default": false,
			"readOnly": true
		}
	}
})"_json;

int main() {
	json_validator validator(read_only_schema);
	try {
		validator.validate(R"({
			"debug": true
		})"_json);
	} catch (const std::exception&e ) {
		return EXIT_SUCCESS;
	}
	
	return EXIT_FAILURE;
}

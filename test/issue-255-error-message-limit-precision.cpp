#include <nlohmann/json-schema.hpp>
#include <stdexcept>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static const json schema = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "$id": "arc.schema.json",
  "properties": {
    "angle": {
      "type": "number",
      "description": "Radians, from -π to π.",
      "minimum": -3.14159265358979323846,
      "maximum": 3.14159265358979323846
    }
  }
})"_json;

class custom_error_handler : public nlohmann::json_schema::basic_error_handler
{
	void error(const nlohmann::json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
		if (message != "instance exceeds maximum of 3.141592653589793")
			throw std::invalid_argument("Precision print does not work.");
	}
};

int main(void)
{
	json_validator validator;

	auto instance = R"({ "angle": 3.1415927410125732 })"_json;

	validator.set_root_schema(schema);
	custom_error_handler err;
	validator.validate(instance, err);

	return 0;
}

#include "nlohmann/json-schema.hpp"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

/*
 * This schema describes a JSON object representing a map from strings to
 * complex numbers, where each complex numbers is a 2-element array of
 * numbers.
 */
static auto schema = json::parse(R"JSON({
	"description": "map from from string to complex numbers",
	"additionalProperties": {
		"description": "complex number",
		"type": "array",
		"items": [
			{
				"description": "real part",
				"type": "number"
			},
			{
				"description": "imaginary part",
				"type": "number"
			}
		]
	}
})JSON");

/*
 * This example contains 4 errors in total.
 * 
 * Additional properties had previously discarded all but the first
 * error message from the "additionalProperties" schema validation.
 * The value of "invalid-real-and-imaginary-part" would thus
 * only emit an error for the invalid real part and the error
 * for the invalid imaginary part was quietly dicarded.
 */
static auto example = json::parse(R"JSON({
	"valid": [1, 2],
	"invalid-imaginary-part": [1, "invalid"],
	"invalid-real-part": ["invalid", 2],
	"invalid-real-and-imaginary-part": ["invalid", "invalid"]
})JSON");

class counting_error_handler : public nlohmann::json_schema::error_handler {
	size_t total_ = 0;

public:
	void error(const json::json_pointer &, const json &, const std::string &) override
	{
		total_ += 1;
	}

	size_t total() {
		return total_;
	}
};

int main(void)
{
	auto validator = json_validator(schema);
	auto counter = counting_error_handler();
	validator.validate(example, counter);

	if (counter.total() != 4) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

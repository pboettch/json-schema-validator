#include "nlohmann/json-schema.hpp"

int main(void)
{
	using nlohmann::json_schema::json_validator;

	json_validator({{"type", "number"}, {"multipleOf", 0.001}}).validate(0.3 - 0.2);

	json_validator({{"type", "number"}, {"multipleOf", 3.3}}).validate(8.0 - 1.4);

	json_validator({{"type", "number"}, {"multipleOf", 1000.01}}).validate((1000.03 - 0.02) * 15.0);

	int expect_exception = 2;
	try {
		json_validator({{"type", "number"}, {"multipleOf", 0.001}}).validate(0.3 - 0.2005);
	} catch (const std::exception &ex) {
		expect_exception--;
	}

	try {
		json_validator({{"type", "number"}, {"multipleOf", 1000.02}}).validate((1000.03 - 0.02) * 15.0);
	} catch (const std::exception &ex) {
		expect_exception--;
	}

	return expect_exception;
}

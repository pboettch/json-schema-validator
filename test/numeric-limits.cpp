#include <nlohmann/json-schema.hpp>

#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static int failures = 0;

static void expect(const json &schema, const json &instance, bool valid)
{
	json_validator validator(schema);

	bool is_valid = true;
	try {
		validator.validate(instance);
	} catch (const std::exception &) {
		is_valid = false;
	}

	if (is_valid != valid) {
		std::cerr << instance.dump() << " should be " << (valid ? "valid" : "invalid") << " for " << schema.dump() << "\n";
		failures++;
	}
}

int main(void)
{
	const json u63 = json::parse("9223372036854775808");  // 2^63, stored as unsigned
	const json u64 = json::parse("18446744073709551615"); // 2^64 - 1
	const json i63 = json::parse("-9223372036854775808"); // -2^63
	const json p53 = json::parse("9007199254740993");     // 2^53 + 1, not a double

	// unsigned integers above INT64_MAX, as instance and as bound
	expect({{"type", "integer"}, {"minimum", 0}}, u63, true);
	expect({{"type", "integer"}, {"minimum", 0}}, u64, true);
	expect({{"type", "integer"}, {"maximum", u64}}, 0, true);
	expect({{"type", "integer"}, {"maximum", u64}}, u64, true);
	expect({{"type", "integer"}, {"exclusiveMaximum", u64}}, u64, false);
	expect({{"type", "integer"}, {"maximum", u63}}, json::parse("9223372036854775807"), true);
	expect({{"type", "integer"}, {"maximum", u63}}, json::parse("9223372036854775809"), false);
	expect({{"type", "integer"}, {"maximum", -1}}, u63, false);
	expect({{"type", "integer"}, {"minimum", u63}}, i63, false);
	expect({{"type", "integer"}, {"minimum", i63}}, i63, true);
	expect({{"type", "integer"}, {"exclusiveMinimum", i63}}, i63, false);

	// fractional bounds of integers
	expect({{"type", "integer"}, {"minimum", 0.5}}, 0, false);
	expect({{"type", "integer"}, {"minimum", 0.5}}, 1, true);
	expect({{"type", "integer"}, {"maximum", 1.5}}, 2, false);
	expect({{"type", "integer"}, {"maximum", -0.5}}, 0, false);
	expect({{"type", "integer"}, {"maximum", -0.5}}, -1, true);
	expect({{"type", "integer"}, {"exclusiveMaximum", 1.0}}, 1, false);
	expect({{"type", "integer"}, {"exclusiveMinimum", 0.999}}, 1, true);

	// integers above 2^53 compared with integer and floating point bounds
	expect({{"type", "number"}, {"maximum", 9007199254740992}}, p53, false);
	expect({{"type", "number"}, {"maximum", 9007199254740992.0}}, p53, false);
	expect({{"type", "number"}, {"exclusiveMaximum", p53}}, 9007199254740992.0, true);
	expect({{"type", "integer"}, {"minimum", p53}}, 9007199254740992, false);
	expect({{"type", "number"}, {"maximum", 1e300}}, u64, true);
	expect({{"type", "number"}, {"minimum", -1e300}}, i63, true);
	expect({{"type", "number"}, {"maximum", 1.5}}, 1.5, true);
	expect({{"type", "number"}, {"maximum", 1}}, 1.5, false);

	// a bound and its exclusive variant together, the stricter one decides
	expect({{"type", "integer"}, {"maximum", 5}, {"exclusiveMaximum", 10}}, 7, false);
	expect({{"type", "integer"}, {"maximum", 10}, {"exclusiveMaximum", 5}}, 7, false);
	expect({{"type", "integer"}, {"maximum", 10}, {"exclusiveMaximum", 5}}, 4, true);
	expect({{"type", "number"}, {"minimum", 5}, {"exclusiveMinimum", 1}}, 3, false);
	expect({{"type", "number"}, {"minimum", 1}, {"exclusiveMinimum", 5}}, 3, false);
	expect({{"type", "number"}, {"minimum", 1}, {"exclusiveMinimum", 5}}, 6, true);

	// multipleOf of integers above 2^53
	expect({{"type", "integer"}, {"multipleOf", 2}}, json::parse("4611686018427387905"), false);
	expect({{"type", "integer"}, {"multipleOf", 2}}, json::parse("4611686018427387906"), true);
	expect({{"type", "integer"}, {"multipleOf", 3}}, json::parse("9223372036854775810"), false);
	expect({{"type", "integer"}, {"multipleOf", 3}}, json::parse("9223372036854775809"), true);
	expect({{"type", "integer"}, {"multipleOf", 3}}, json::parse("-9223372036854775806"), true);
	expect({{"type", "integer"}, {"multipleOf", 3}}, json::parse("-9223372036854775807"), false);
	expect({{"type", "integer"}, {"multipleOf", 2}}, i63, true);
	expect({{"type", "integer"}, {"multipleOf", 7}}, 49, true);
	expect({{"type", "integer"}, {"multipleOf", 7}}, -48, false);
	expect({{"type", "integer"}, {"multipleOf", 0.5}}, 3, true);

	// a bound has to be a number
	int thrown = 0;
	for (auto keyword : {"maximum", "minimum", "exclusiveMaximum", "exclusiveMinimum"}) {
		try {
			json_validator validator(json{{"type", "number"}, {keyword, true}});
		} catch (const std::invalid_argument &) {
			thrown++;
		}
	}
	if (thrown != 4) {
		std::cerr << "a boolean bound should be refused\n";
		failures++;
	}

	return failures;
}

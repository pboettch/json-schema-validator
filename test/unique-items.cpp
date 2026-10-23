#include <nlohmann/json-schema.hpp>

#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static int failures = 0;

static void expect(const json &instance, bool valid)
{
	json_validator validator(json{{"uniqueItems", true}});

	bool is_valid = true;
	try {
		validator.validate(instance);
	} catch (const std::exception &) {
		is_valid = false;
	}

	if (is_valid != valid) {
		std::cerr << instance.dump() << " should be " << (valid ? "valid" : "invalid") << "\n";
		failures++;
	}
}

int main(void)
{
	expect(json::array(), true);
	expect({1, 2, 3}, true);
	expect({1, 2, 1}, false);
	expect({3, 2, 1, 2}, false);
	expect({1, 1.0}, false);
	expect({1, true}, true);
	expect({0, false, nullptr, "0", json::array(), json::object()}, true);
	expect({nullptr, nullptr}, false);
	expect({"a", "b", "a"}, false);
	expect({{1, 2}, {2, 1}}, true);
	expect({{1, 2}, {1, 2.0}}, false);
	expect({{1, 2}, {1, 2, 3}}, true);
	expect(json::parse(R"([{"a": 1, "b": 2}, {"b": 2, "a": 1.0}])"), false);
	expect(json::parse(R"([{"a": 1}, {"b": 1}])"), true);
	expect(json::parse(R"([{"a": 1}, {"a": 1, "b": 1}])"), true);

	// integers above 2^53 are compared exactly, also with floating point numbers
	expect(json::parse("[9007199254740993, 9007199254740992.0]"), true);
	expect(json::parse("[9007199254740992, 9007199254740992.0]"), false);
	expect(json::parse("[9223372036854775808, -9223372036854775808]"), true);
	expect(json::parse("[18446744073709551615, 18446744073709551615]"), false);
	expect(json::parse("[0.5, 0, 1]"), true);
	expect(json::parse("[-0.5, -1, 0]"), true);
	expect(json::parse("[1e300, -1e300, 18446744073709551615, -9223372036854775808]"), true);

	// n log n: n^2 comparisons of this array take minutes
	json large = json::array();
	for (int i = 0; i < 200000; i++)
		large.push_back(i);
	expect(large, true);
	large.push_back(100000);
	expect(large, false);

	return failures;
}

#include <nlohmann/json-schema.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static int error_count;

#define EXPECT_EQ(a, b)                                              \
	do {                                                             \
		if ((a) != (b)) {                                            \
			std::cerr << "Failed: '" << a << "' != '" << b << "'\n"; \
			error_count++;                                           \
		}                                                            \
	} while (0)

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::basic_error_handler;
using nlohmann::json_schema::error_handler;
using nlohmann::json_schema::json_validator;
using nlohmann::json_schema::validation_error;

namespace
{

class collecting_error_handler : public error_handler
{
	void error(const validation_error &error) override
	{
		errors.push_back(error);
	}

public:
	std::vector<validation_error> errors;
};

void expect_single_error(const json &schema, const json &instance, const std::string &keyword, const json &details)
{
	json_validator validator(schema);
	collecting_error_handler errors;
	validator.validate(instance, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() != 1)
		return;
	EXPECT_EQ(errors.errors[0].keyword, keyword);
	EXPECT_EQ(errors.errors[0].details, details);
}

void test_scalar_keyword_details()
{
	expect_single_error({{"type", "integer"}}, "value", "type", {{"value", "integer"}, {"actual_type", "string"}});
	expect_single_error({{"minLength", 2}}, "a", "minLength", {{"value", 2}});
	expect_single_error({{"maxLength", 2}}, "abc", "maxLength", {{"value", 2}});
	expect_single_error({{"pattern", "^a"}}, "bad", "pattern", {{"value", "^a"}});
	expect_single_error({{"multipleOf", 2}}, 3, "multipleOf", {{"value", 2}});
	expect_single_error({{"maximum", 2}}, 3, "maximum", {{"value", 2}});
	expect_single_error({{"exclusiveMaximum", 2}}, 2, "exclusiveMaximum", {{"value", 2}});
	expect_single_error({{"minimum", 2}}, 1, "minimum", {{"value", 2}});
	expect_single_error({{"exclusiveMinimum", 2}}, 2, "exclusiveMinimum", {{"value", 2}});
}

void test_multiple_scalar_failures()
{
	const json schema = {{"multipleOf", 2}, {"maximum", 3}};
	json_validator validator(schema);
	collecting_error_handler errors;
	validator.validate(5, errors);
	EXPECT_EQ(errors.errors.size(), 2);
	if (errors.errors.size() != 2)
		return;
	EXPECT_EQ(errors.errors[0].keyword, "multipleOf");
	EXPECT_EQ(errors.errors[1].keyword, "maximum");
}

void test_format_keyword_details()
{
	const json schema = {{"type", "string"}, {"format", "custom"}};
	auto checker = [](const std::string &, const std::string &) { throw std::invalid_argument("bad format"); };
	json_validator validator(schema, nullptr, checker);
	collecting_error_handler errors;
	validator.validate("value", errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].keyword, "format");
		EXPECT_EQ(errors.errors[0].details, json({{"value", "custom"}, {"reason", "bad format"}}));
	}
}

void test_content_keyword_details()
{
	const json schema = {{"type", "string"}, {"contentEncoding", "base64"}, {"contentMediaType", "text/plain"}};
	auto checker = [](const std::string &, const std::string &, const json &) { throw std::invalid_argument("bad content"); };
	json_validator validator(schema, nullptr, nullptr, checker);
	collecting_error_handler errors;
	validator.validate("value", errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].keyword, "contentEncoding");
		EXPECT_EQ(errors.errors[0].details, json({{"value", "base64"}, {"content_media_type", "text/plain"}, {"reason", "bad content"}}));
	}
}

void test_array_keyword_details()
{
	expect_single_error({{"type", "array"}, {"minItems", 2}}, {1}, "minItems", {{"value", 2}});
	expect_single_error({{"type", "array"}, {"maxItems", 1}}, {1, 2}, "maxItems", {{"value", 1}});
	expect_single_error({{"type", "array"}, {"uniqueItems", true}}, {1, 1}, "uniqueItems", {{"value", true}, {"duplicate", 1}});
	expect_single_error({{"type", "array"}, {"contains", {{"const", "required"}}}}, {"other"}, "contains", json::object());
	expect_single_error({{"type", "array"}, {"items", {{{"type", "integer"}}}}, {"additionalItems", false}}, {1, 2}, "additionalItems", {{"code", "false-schema"}, {"value", false}});
	expect_single_error({{"type", "array"}, {"items", {{{"type", "integer"}}}}, {"additionalItems", {{"minimum", 10}}}}, {1, 2}, "minimum", {{"value", 10}});
}

void test_tuple_item_locations()
{
	collecting_error_handler errors;
	json_validator items_validator({{"type", "array"}, {"items", {{{"type", "integer"}}, {{"type", "integer"}}}}});
	items_validator.validate({1, "bad"}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1)
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/1"));

	errors.errors.clear();
	json_validator additional_validator({{"type", "array"}, {"items", {{{"type", "integer"}}}}, {"additionalItems", false}});
	additional_validator.validate({1, 2}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1)
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/1"));
}

void test_not_keyword_details()
{
	expect_single_error({{"not", {{"const", "bad"}}}}, "bad", "not", json::object());
}

void test_object_keyword_details()
{
	expect_single_error({{"type", "object"}, {"minProperties", 2}}, {{"a", 1}}, "minProperties", {{"value", 2}});
	expect_single_error({{"type", "object"}, {"maxProperties", 1}}, {{"a", 1}, {"b", 2}}, "maxProperties", {{"value", 1}});
	expect_single_error({{"type", "object"}, {"required", {"a", "b"}}}, {{"a", 1}}, "required", {{"value", {"a", "b"}}, {"missing_property", "b"}});
	expect_single_error({{"type", "object"}, {"dependencies", {{"credit_card", {"billing_address"}}}}}, {{"credit_card", 1}}, "dependencies", {{"value", {"billing_address"}}, {"property", "credit_card"}, {"missing_property", "billing_address"}});
	expect_single_error({{"type", "object"}, {"additionalProperties", false}}, {{"extra", 1}}, "additionalProperties", {{"property", "extra"}, {"value", false}, {"code", "false-schema"}});
	expect_single_error({{"type", "object"}, {"additionalProperties", {{"type", "object"}, {"additionalProperties", false}}}}, {{"outer", {{"inner", 1}}}}, "additionalProperties", {{"property", "inner"}, {"value", false}, {"code", "false-schema"}});
	expect_single_error({{"type", "object"}, {"propertyNames", false}}, {{"bad", 1}}, "propertyNames", {{"property", "bad"}, {"value", false}, {"code", "false-schema"}});
}

void test_direct_keyword_details()
{
	const json schema = {
	    {"type", "object"},
	    {"properties",
	     {
	         {"color", {{"enum", {"red", "green"}}}},
	         {"mode", {{"const", "fast"}}},
	         {"count", {{"type", "integer"}}},
	     }},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate({{"color", "blue"}, {"mode", "slow"}, {"count", "many"}}, errors);
	EXPECT_EQ(errors.errors.size(), 3);
	if (errors.errors.size() != 3)
		return;

	EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/color"));
	EXPECT_EQ(errors.errors[0].instance, json("blue"));
	EXPECT_EQ(errors.errors[0].message, "instance not found in required enum");
	EXPECT_EQ(errors.errors[0].keyword, "enum");
	EXPECT_EQ(errors.errors[0].details, json({{"value", json({"red", "green"})}}));

	EXPECT_EQ(errors.errors[1].instance_location, json::json_pointer("/count"));
	EXPECT_EQ(errors.errors[1].keyword, "type");
	EXPECT_EQ(errors.errors[1].details, json({{"value", "integer"}, {"actual_type", "string"}}));

	EXPECT_EQ(errors.errors[2].instance_location, json::json_pointer("/mode"));
	EXPECT_EQ(errors.errors[2].instance, json("slow"));
	EXPECT_EQ(errors.errors[2].message, "instance not const");
	EXPECT_EQ(errors.errors[2].keyword, "const");
	EXPECT_EQ(errors.errors[2].details, json({{"value", json("fast")}}));
}

void test_details_survive_logical_combinations()
{
	const json schema = {
	    {"oneOf", {{{"const", "Point"}}, {{"const", "Linear"}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate("Log", errors);
	EXPECT_EQ(errors.errors.size(), 3);
	if (errors.errors.size() != 3)
		return;

	EXPECT_EQ(errors.errors[0].keyword, "oneOf");
	EXPECT_EQ(errors.errors[0].details, json({{"failed_subschemas", 2}}));

	EXPECT_EQ(errors.errors[1].keyword, "const");
	EXPECT_EQ(errors.errors[1].details, json({{"value", json("Point")}}));
	EXPECT_EQ(errors.errors[1].message, "[combination: oneOf / case#0] instance not const");

	EXPECT_EQ(errors.errors[2].keyword, "const");
	EXPECT_EQ(errors.errors[2].details, json({{"value", json("Linear")}}));
	EXPECT_EQ(errors.errors[2].message, "[combination: oneOf / case#1] instance not const");
}

void test_one_of_multiple_matches()
{
	const json schema = {
	    {"oneOf", {{{"type", "number"}}, {{"minimum", 0}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate(1, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() != 1)
		return;

	EXPECT_EQ(errors.errors[0].keyword, "oneOf");
	EXPECT_EQ(errors.errors[0].details, json({{"successful_subschemas", 2}}));
}

void test_details_survive_any_of()
{
	const json schema = {
	    {"anyOf", {{{"const", "Point"}}, {{"const", "Linear"}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate("Log", errors);
	EXPECT_EQ(errors.errors.size(), 3);
	if (errors.errors.size() != 3)
		return;

	EXPECT_EQ(errors.errors[0].keyword, "anyOf");
	EXPECT_EQ(errors.errors[0].details, json({{"failed_subschemas", 2}}));

	EXPECT_EQ(errors.errors[1].keyword, "const");
	EXPECT_EQ(errors.errors[1].details, json({{"value", json("Point")}}));
	EXPECT_EQ(errors.errors[1].message, "[combination: anyOf / case#0] instance not const");

	EXPECT_EQ(errors.errors[2].keyword, "const");
	EXPECT_EQ(errors.errors[2].details, json({{"value", json("Linear")}}));
	EXPECT_EQ(errors.errors[2].message, "[combination: anyOf / case#1] instance not const");
}

void test_details_survive_all_of()
{
	const json schema = {
	    {"allOf", {{{"const", "Point"}}, {{"const", "Linear"}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate("Log", errors);
	EXPECT_EQ(errors.errors.size(), 2);
	if (errors.errors.size() != 2)
		return;

	EXPECT_EQ(errors.errors[0].keyword, "allOf");
	EXPECT_EQ(errors.errors[0].details, json({{"failed_subschema", 0}}));

	EXPECT_EQ(errors.errors[1].keyword, "const");
	EXPECT_EQ(errors.errors[1].details, json({{"value", json("Point")}}));
	EXPECT_EQ(errors.errors[1].message, "[combination: allOf / case#0] instance not const");
}

void test_details_survive_nested_combinations()
{
	const json schema = {
	    {"anyOf",
	     {
	         {{"oneOf", {{{"const", "Point"}}, {{"const", "Linear"}}}}},
	         {{"const", "Log"}},
	     }},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate("Other", errors);
	EXPECT_EQ(errors.errors.size(), 5);
	if (errors.errors.size() != 5)
		return;

	EXPECT_EQ(errors.errors[0].keyword, "anyOf");
	EXPECT_EQ(errors.errors[0].details, json({{"failed_subschemas", 2}}));

	EXPECT_EQ(errors.errors[2].keyword, "const");
	EXPECT_EQ(errors.errors[2].details, json({{"value", json("Point")}}));
	EXPECT_EQ(errors.errors[2].message, "[combination: anyOf / case#0] [combination: oneOf / case#0] instance not const");

	EXPECT_EQ(errors.errors[3].keyword, "const");
	EXPECT_EQ(errors.errors[3].details, json({{"value", json("Linear")}}));

	EXPECT_EQ(errors.errors[4].keyword, "const");
	EXPECT_EQ(errors.errors[4].details, json({{"value", json("Log")}}));
}

void test_details_survive_additional_properties()
{
	const json schema = {
	    {"type", "object"},
	    {"additionalProperties", {{"enum", {"red", "green"}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate({{"color", "blue"}}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() != 1)
		return;

	EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer(""));
	EXPECT_EQ(errors.errors[0].message, "validation failed for additional property 'color': instance not found in required enum");
	EXPECT_EQ(errors.errors[0].keyword, "enum");
	EXPECT_EQ(errors.errors[0].details, json({{"value", json({"red", "green"})}, {"property", "color"}}));
}

void test_non_keyword_error_details()
{
	collecting_error_handler errors;
	json_validator empty_validator;
	empty_validator.validate(nullptr, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].keyword, "");
		EXPECT_EQ(errors.errors[0].details, json({{"code", "no-root-schema"}}));
	}

	expect_single_error(false, nullptr, "", {{"code", "false-schema"}});

	json_validator validator(true);
	errors.errors.clear();
	validator.validate(nullptr, errors, json_uri("#/missing"));
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1)
		EXPECT_EQ(errors.errors[0].details, json({{"code", "schema-not-found"}, {"location", ""}, {"fragment", "/missing"}}));

	errors.errors.clear();
	validator.validate(nullptr, errors, json_uri("missing.json#"));
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1)
		EXPECT_EQ(errors.errors[0].details, json({{"code", "schema-file-not-found"}, {"location", "/missing.json"}}));
}

void test_basic_error_handler()
{
	const json schema = {{"enum", {"red", "green"}}};
	json_validator validator(schema);
	basic_error_handler errors;

	validator.validate("blue", errors);
	EXPECT_EQ(static_cast<bool>(errors), true);
	errors.reset();
	EXPECT_EQ(static_cast<bool>(errors), false);
}

} // namespace

int main()
{
	test_scalar_keyword_details();
	test_multiple_scalar_failures();
	test_format_keyword_details();
	test_content_keyword_details();
	test_array_keyword_details();
	test_tuple_item_locations();
	test_not_keyword_details();
	test_object_keyword_details();
	test_direct_keyword_details();
	test_details_survive_logical_combinations();
	test_one_of_multiple_matches();
	test_details_survive_any_of();
	test_details_survive_all_of();
	test_details_survive_nested_combinations();
	test_details_survive_additional_properties();
	test_non_keyword_error_details();
	test_basic_error_handler();
	return error_count;
}

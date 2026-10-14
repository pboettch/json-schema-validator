#include <nlohmann/json-schema.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
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

struct recorded_validation_error {
	json::json_pointer instance_location;
	json instance;
	std::string message;
	std::string keyword;
	json details;
};

class collecting_error_handler : public error_handler
{
	void error(const validation_error &error, const json &instance) override
	{
		errors.push_back({error.instance_location, instance, error.message, error.keyword, error.details});
	}

public:
	std::vector<recorded_validation_error> errors;
};

class incomplete_error_handler : public error_handler
{
};

class legacy_error_handler : public basic_error_handler
{
	void error(const json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
		basic_error_handler::error(ptr, instance, message);
		location = ptr.to_string();
		reported_instance = instance;
		reported_message = message;
		count++;
	}

public:
	int count{0};
	std::string location;
	json reported_instance;
	std::string reported_message;
};

void expect_single_error(const json &schema,
                         const json &instance,
                         const std::string &keyword,
                         const json &details,
                         const std::string &message = {})
{
	json_validator validator(schema);
	collecting_error_handler errors;
	validator.validate(instance, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() != 1)
		return;
	EXPECT_EQ(errors.errors[0].keyword, keyword);
	EXPECT_EQ(errors.errors[0].details, details);
	if (!message.empty())
		EXPECT_EQ(errors.errors[0].message, message);
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
	expect_single_error({{"type", "array"}, {"items", {{{"type", "integer"}}}}, {"additionalItems", false}}, {1, 2}, "additionalItems", {{"value", false}}, "unexpected additional item");
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
	expect_single_error({{"type", "object"}, {"required", {"a", "b"}}}, {{"a", 1}}, "required", {{"missing_property", "b"}});
	expect_single_error({{"type", "object"}, {"dependencies", {{"credit_card", {"billing_address"}}}}}, {{"credit_card", 1}}, "dependencies", {{"property", "credit_card"}, {"missing_property", "billing_address"}});
	expect_single_error({{"type", "object"}, {"additionalProperties", false}}, {{"extra", 1}}, "additionalProperties", {{"value", false}}, "unexpected additional property 'extra'");
	expect_single_error({{"type", "object"}, {"additionalProperties", {{"type", "object"}, {"additionalProperties", false}}}}, {{"outer", {{"inner", 1}}}}, "additionalProperties", {{"value", false}}, "unexpected additional property 'inner'");
	expect_single_error({{"type", "object"}, {"propertyNames", false}}, {{"bad", 1}}, "propertyNames", {{"property", "bad"}, {"value", false}}, "invalid property name 'bad'");
}

void test_allowed_boolean_applicators()
{
	collecting_error_handler errors;
	json_validator object_validator({{"type", "object"}, {"propertyNames", true}, {"additionalProperties", true}});
	object_validator.validate({{"extra", 1}}, errors);
	EXPECT_EQ(errors.errors.size(), 0);

	json_validator array_validator({{"type", "array"}, {"items", {{{"type", "integer"}}}}, {"additionalItems", true}});
	array_validator.validate({1, 2}, errors);
	EXPECT_EQ(errors.errors.size(), 0);
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
	EXPECT_EQ(errors.errors.size(), 4);
	if (errors.errors.size() != 4)
		return;

	// every failed branch is reported: its summary, then its errors
	EXPECT_EQ(errors.errors[0].keyword, "allOf");
	EXPECT_EQ(errors.errors[0].details, json({{"failed_subschema", 0}}));

	EXPECT_EQ(errors.errors[1].keyword, "const");
	EXPECT_EQ(errors.errors[1].details, json({{"value", json("Point")}}));
	EXPECT_EQ(errors.errors[1].message, "[combination: allOf / case#0] instance not const");

	EXPECT_EQ(errors.errors[2].keyword, "allOf");
	EXPECT_EQ(errors.errors[2].details, json({{"failed_subschema", 1}}));

	EXPECT_EQ(errors.errors[3].keyword, "const");
	EXPECT_EQ(errors.errors[3].details, json({{"value", json("Linear")}}));
	EXPECT_EQ(errors.errors[3].message, "[combination: allOf / case#1] instance not const");

	// a branch which validates is not reported
	const json partial_schema = {{"allOf", {{{"const", "Point"}}, {{"type", "string"}}, {{"const", "Linear"}}}}};
	json_validator partial(partial_schema);
	errors.errors.clear();
	partial.validate("Log", errors);
	EXPECT_EQ(errors.errors.size(), 4);
	if (errors.errors.size() == 4)
		EXPECT_EQ(errors.errors[2].details, json({{"failed_subschema", 2}}));
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

	EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/color"));
	EXPECT_EQ(errors.errors[0].message, "instance not found in required enum");
	EXPECT_EQ(errors.errors[0].keyword, "enum");
	EXPECT_EQ(errors.errors[0].details, json({{"value", json({"red", "green"})}}));
}

void test_all_additional_property_errors()
{
	const json schema = {
	    {"type", "object"},
	    {"additionalProperties", {{"type", "object"}, {"required", {"first", "second"}}}},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	validator.validate({{"outer", json::object()}}, errors);
	EXPECT_EQ(errors.errors.size(), 2);
	if (errors.errors.size() != 2)
		return;

	EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/outer"));
	EXPECT_EQ(errors.errors[0].keyword, "required");
	EXPECT_EQ(errors.errors[0].details.at("missing_property"), "first");
	EXPECT_EQ(errors.errors[1].instance_location, json::json_pointer("/outer"));
	EXPECT_EQ(errors.errors[1].keyword, "required");
	EXPECT_EQ(errors.errors[1].details.at("missing_property"), "second");
}

void test_nested_additional_property_context()
{
	collecting_error_handler errors;
	json_validator deny_validator({{"type", "object"}, {"additionalProperties", false}});
	deny_validator.validate({{"extra", 1}}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/extra"));
		EXPECT_EQ(errors.errors[0].instance, 1);
		EXPECT_EQ(errors.errors[0].keyword, "additionalProperties");
		EXPECT_EQ(errors.errors[0].details, json({{"value", false}}));
	}

	errors.errors.clear();
	json_validator reference_validator({
	    {"type", "object"},
	    {"definitions", {{"deny", false}}},
	    {"additionalProperties", {{"$ref", "#/definitions/deny"}}},
	});
	reference_validator.validate({{"extra", 1}}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/extra"));
		EXPECT_EQ(errors.errors[0].instance, 1);
		EXPECT_EQ(errors.errors[0].keyword, "");
		EXPECT_EQ(errors.errors[0].details, json({{"code", "false-schema"}}));
	}

	errors.errors.clear();
	json_validator nested_false_validator({
	    {"type", "object"},
	    {"additionalProperties", {{"type", "object"}, {"properties", {{"x", false}}}}},
	});
	nested_false_validator.validate({{"outer", {{"x", 1}}}}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/outer/x"));
		EXPECT_EQ(errors.errors[0].instance, 1);
		EXPECT_EQ(errors.errors[0].keyword, "");
		EXPECT_EQ(errors.errors[0].details, json({{"code", "false-schema"}}));
	}

	errors.errors.clear();
	json_validator property_name_validator({
	    {"type", "object"},
	    {"additionalProperties", {{"type", "object"}, {"propertyNames", false}}},
	});
	property_name_validator.validate({{"foo", {{"bar", "baz"}}}}, errors);
	EXPECT_EQ(errors.errors.size(), 1);
	if (errors.errors.size() == 1) {
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer("/foo"));
		EXPECT_EQ(errors.errors[0].instance, "bar");
		EXPECT_EQ(errors.errors[0].keyword, "propertyNames");
		EXPECT_EQ(errors.errors[0].details, json({{"value", false}, {"property", "bar"}}));
	}
}

// propertyNames validates a json built from the property key, which is not part of the instance
// document. Errors reaching the handler through a logical combination must still be delivered
// with a live instance (regression: stack-use-after-return when errors were buffered).
void test_property_names_inside_logical_combinations()
{
	const json instance = {{"toolong", 1}};

	struct case_t {
		json schema;
		std::string keyword;
		size_t expected_errors;
	};
	const std::vector<case_t> cases = {
	    {{{"oneOf", {{{"type", "object"}, {"propertyNames", false}}, {{"type", "string"}}}}}, "propertyNames", 3},
	    {{{"anyOf", {{{"type", "object"}, {"propertyNames", {{"maxLength", 2}}}}, {{"type", "string"}}}}}, "maxLength", 3},
	    {{{"allOf", {{{"type", "object"}, {"propertyNames", {{"pattern", "^x"}}}}}}}, "pattern", 2},
	};

	for (const auto &c : cases) {
		json_validator validator(c.schema);
		collecting_error_handler errors;
		validator.validate(instance, errors);
		EXPECT_EQ(errors.errors.size(), c.expected_errors);
		if (errors.errors.size() != c.expected_errors)
			continue;
		size_t found = 0;
		for (const auto &propagated : errors.errors) {
			if (propagated.keyword != c.keyword)
				continue;
			found++;
			EXPECT_EQ(propagated.instance, json("toolong"));
			EXPECT_EQ(propagated.details.at("property"), "toolong");
		}
		EXPECT_EQ(found, 1);
		EXPECT_EQ(errors.errors.front().instance, instance); // the summary belongs to the combination
	}

	// the same through $ref and additionalProperties, two levels down
	json_validator nested({
	    {"definitions", {{"deny", false}}},
	    {"allOf", {{{"additionalProperties", {{"propertyNames", {{"$ref", "#/definitions/deny"}}}}}}}},
	});
	collecting_error_handler errors;
	nested.validate({{"outer", {{"bad", 0}}}}, errors);
	EXPECT_EQ(errors.errors.size(), 2);
	if (errors.errors.size() == 2) {
		EXPECT_EQ(errors.errors[0].keyword, "allOf");
		EXPECT_EQ(errors.errors[0].instance_location, json::json_pointer(""));
		EXPECT_EQ(errors.errors[0].message, "at least one subschema has failed, but all of them are required to validate");
		EXPECT_EQ(errors.errors[0].details, json({{"failed_subschema", 0}}));
		EXPECT_EQ(errors.errors[1].instance_location, json::json_pointer("/outer"));
		EXPECT_EQ(errors.errors[1].instance, json("bad"));
		EXPECT_EQ(errors.errors[1].details, json({{"code", "false-schema"}, {"property", "bad"}}));
	}
}

// The probe's verdict is authoritative. A checker whose outcome changes between the probe and
// the diagnostics pass must not turn an established failure into silent success.
void test_combination_failure_is_reported_even_if_reevaluation_passes()
{
	static int calls;
	calls = 0;
	auto flaky = [](const std::string &, const std::string &) {
		if (calls++ == 0)
			throw std::invalid_argument("rejected once");
	};
	const json schema = {{"allOf", {{{"format", "flaky"}}}}};

	json_validator validator(schema, nullptr, flaky);
	basic_error_handler failed;
	validator.validate("value", failed);
	EXPECT_EQ(static_cast<bool>(failed), true);

	calls = 0;
	json_validator throwing(schema, nullptr, flaky);
	bool threw = false;
	try {
		throwing.validate("value");
	} catch (const std::invalid_argument &) {
		threw = true;
	}
	EXPECT_EQ(threw, true);
}

// Probing is visible through forwarding handlers (here propertyNames) and to the default
// throwing handler, so a subschema is evaluated exactly once when no diagnostics are produced.
void test_probe_evaluates_once()
{
	static int calls;
	auto reject = [](const std::string &, const std::string &) {
		calls++;
		throw std::invalid_argument("rejected");
	};

	// failing branch behind propertyNames inside a successful anyOf: no diagnostics wanted
	calls = 0;
	json_validator any_of({{"anyOf", {{{"propertyNames", {{"allOf", {{{"format", "reject"}}}}}}}, json::object()}}}, nullptr, reject);
	basic_error_handler failed;
	any_of.validate({{"a", 0}}, failed);
	EXPECT_EQ(static_cast<bool>(failed), false);
	EXPECT_EQ(calls, 1);

	// three nested allOf; the throwing handler stops at the outermost summary
	calls = 0;
	json_validator nested({{"allOf", {{{"allOf", {{{"allOf", {{{"format", "reject"}}}}}}}}}}}, nullptr, reject);
	bool threw = false;
	try {
		nested.validate("value");
	} catch (const std::invalid_argument &) {
		threw = true;
	}
	EXPECT_EQ(threw, true);
	EXPECT_EQ(calls, 1);
}

// A checker running inside a probe may validate with another validator; that validation is its
// own evaluation and must deliver full diagnostics to its own handler, and the enclosing probe
// must resume afterwards, also when the inner validation throws.
void test_reentrant_validation_inside_probe()
{
	static std::vector<recorded_validation_error> inner_errors;
	static int reject_calls;
	inner_errors.clear();
	reject_calls = 0;

	auto checker = [](const std::string &format, const std::string &) {
		if (format == "reject") {
			reject_calls++;
			throw std::invalid_argument("rejected");
		}
		json_validator inner({{"enum", {1, 2}}, {"anyOf", {{{"const", 1}}, {{"const", 2}}}}});
		collecting_error_handler errors;
		inner.validate(0, errors);
		inner_errors = errors.errors;
		try {
			inner.validate(0); // throwing handler: unwinds through the inner validation
		} catch (const std::invalid_argument &) {
		}
	};

	// the checker runs while the outer anyOf probes its first branch; the inner allOf that
	// follows must still see the probe (otherwise "reject" would be evaluated a second time)
	json_validator outer({{"anyOf", {{{"allOf", {{{"type", "string"}, {"format", "nested"}}, {{"format", "reject"}}}}}, json::object()}}}, nullptr, checker);
	basic_error_handler failed;
	outer.validate("value", failed);
	EXPECT_EQ(static_cast<bool>(failed), false);
	EXPECT_EQ(reject_calls, 1);

	EXPECT_EQ(inner_errors.size(), 4);
	if (inner_errors.size() == 4) {
		EXPECT_EQ(inner_errors[0].keyword, "enum");
		EXPECT_EQ(inner_errors[0].details, json({{"value", json({1, 2})}}));
		EXPECT_EQ(inner_errors[1].keyword, "anyOf");
		EXPECT_EQ(inner_errors[2].keyword, "const");
		EXPECT_EQ(inner_errors[2].details, json({{"value", 1}}));
		EXPECT_EQ(inner_errors[3].message, "[combination: anyOf / case#1] instance not const");
	}
}

// The probe state must survive a checker that throws something other than std::exception (which
// string::validate does not catch) and a handler that runs another validator while errors are
// being reported: both validations afterwards must deliver full diagnostics.
void test_probe_state_after_unwinding_and_reentrant_handler()
{
	auto throw_int = [](const std::string &, const std::string &) { throw 42; };
	json_validator unwound({{"allOf", {{{"format", "throw-int"}}}}}, nullptr, throw_int);
	bool threw = false;
	try {
		unwound.validate("value");
	} catch (int) {
		threw = true;
	}
	EXPECT_EQ(threw, true);

	static const json inner_schema = {{"anyOf", {{{"const", 2}}, {{"const", 3}}}}};
	json_validator inner(inner_schema);
	collecting_error_handler after_unwind;
	inner.validate(1, after_unwind);
	EXPECT_EQ(after_unwind.errors.size(), 3); // summary and both branches

	// legacy-signature handler which validates something else from inside the callback
	class reentrant_handler : public basic_error_handler
	{
		void error(const json::json_pointer &, const json &, const std::string &) override
		{
			json_validator inner(inner_schema);
			collecting_error_handler errors;
			inner.validate(1, errors);
			inner_error_counts.push_back(errors.errors.size());
		}

	public:
		std::vector<size_t> inner_error_counts;
	};

	const json outer_schema = {{"allOf", {{{"const", 1}}}}};
	json_validator outer(outer_schema);
	reentrant_handler handler;
	outer.validate(0, handler);
	EXPECT_EQ(handler.inner_error_counts.size(), 2); // allOf summary, then the const error
	for (size_t count : handler.inner_error_counts)
		EXPECT_EQ(count, 3);
}

// Failed branches inside a combination that ultimately succeeds must not leak defaults, and
// the diagnostics pass for a failed combination must not add defaults a second time.
void test_combination_default_values()
{
	const json schema = {
	    {"anyOf",
	     {
	         {{"type", "object"}, {"required", {"a"}}, {"properties", {{"x", {{"default", 1}}}}}},
	         {{"type", "object"}, {"required", {"b"}}, {"properties", {{"y", {{"default", 2}}}}}},
	     }},
	};
	json_validator validator(schema);
	collecting_error_handler errors;

	json patch = validator.validate({{"b", 0}}, errors);
	EXPECT_EQ(errors.errors.size(), 0);
	EXPECT_EQ(patch, json::parse(R"([{"op":"add","path":"/y","value":2}])"));

	errors.errors.clear();
	patch = validator.validate(json::object(), errors);
	EXPECT_EQ(errors.errors.size(), 3);
	EXPECT_EQ(patch, json::array());
}

// Draft 7, section 3.3.2: anyOf must not stop at its first successful branch while annotations
// are collected, so the defaults of every successful branch are part of the patch, also when
// the anyOf is itself evaluated inside a probe. is_valid() collects no defaults and stops early.
void test_any_of_collects_defaults_of_every_successful_branch()
{
	const json any_of = {
	    {"anyOf",
	     {
	         {{"properties", {{"x", {{"default", 1}}}}}},
	         {{"required", {"a"}}, {"properties", {{"z", {{"default", 3}}}}}},
	         {{"properties", {{"y", {{"default", 2}}}}}},
	     }},
	};
	const json expected = json::parse(R"([{"op":"add","path":"/x","value":1},{"op":"add","path":"/y","value":2}])");

	json_validator validator(any_of);
	collecting_error_handler errors;
	EXPECT_EQ(validator.validate(json::object(), errors), expected);
	EXPECT_EQ(errors.errors.size(), 0);

	const json nested_schema = {{"oneOf", {any_of, {{"type", "string"}}}}};
	json_validator nested(nested_schema);
	EXPECT_EQ(nested.validate(json::object(), errors), expected);
	EXPECT_EQ(errors.errors.size(), 0);

	static int calls;
	auto count = [](const std::string &, const std::string &) { calls++; };
	json_validator counted({{"anyOf", {json::object(), {{"format", "counted"}}}}}, nullptr, count);
	calls = 0;
	counted.validate("value", errors);
	EXPECT_EQ(calls, 1);
	calls = 0;
	EXPECT_EQ(counted.is_valid("value"), true);
	EXPECT_EQ(calls, 0);
}

void test_validation_error_is_a_value_type()
{
	static_assert(std::is_default_constructible<validation_error>::value, "validation_error must be default-constructible");
	static_assert(std::is_copy_assignable<validation_error>::value, "validation_error must be copy-assignable");
	std::vector<validation_error> errors(2);
	errors.erase(errors.begin());
	EXPECT_EQ(errors.size(), 1);
}

// is_valid() answers with the probe alone: same verdict as a handler, nothing evaluated twice.
void test_is_valid()
{
	static int calls;
	auto reject = [](const std::string &, const std::string &) {
		calls++;
		throw std::invalid_argument("rejected");
	};
	const json nested_schema = {{"allOf", {{{"allOf", {{{"allOf", {{{"format", "reject"}}}}}}}}}}};
	json_validator nested(nested_schema, nullptr, reject);
	calls = 0;
	EXPECT_EQ(nested.is_valid("value"), false);
	EXPECT_EQ(calls, 1);

	const json schema = {{"anyOf", {{{"type", "object"}, {"propertyNames", {{"maxLength", 2}}}}, {{"type", "string"}}}}};
	json_validator validator(schema);
	EXPECT_EQ(validator.is_valid({{"ok", 1}}), true);
	EXPECT_EQ(validator.is_valid({{"toolong", 1}}), false);
	EXPECT_EQ(validator.is_valid("text"), true);

	json_validator empty;
	EXPECT_EQ(empty.is_valid(nullptr), false);
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

void test_incomplete_error_handler()
{
	const json schema = {{"enum", {"red", "green"}}};
	json_validator validator(schema);
	incomplete_error_handler errors;
	bool threw = false;

	try {
		validator.validate("blue", errors);
	} catch (const std::logic_error &error) {
		threw = true;
		EXPECT_EQ(std::string(error.what()), "error_handler must override an error callback");
	}
	EXPECT_EQ(threw, true);
}

void test_legacy_error_handler()
{
	const json schema = {{"enum", {"red", "green"}}};
	json_validator validator(schema);
	legacy_error_handler errors;

	validator.validate("blue", errors);
	EXPECT_EQ(errors.count, 1);
	EXPECT_EQ(errors.location, "");
	EXPECT_EQ(errors.reported_instance, "blue");
	EXPECT_EQ(errors.reported_message.empty(), false);
	EXPECT_EQ(static_cast<bool>(errors), true);
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
	test_allowed_boolean_applicators();
	test_direct_keyword_details();
	test_details_survive_logical_combinations();
	test_one_of_multiple_matches();
	test_details_survive_any_of();
	test_details_survive_all_of();
	test_details_survive_nested_combinations();
	test_details_survive_additional_properties();
	test_all_additional_property_errors();
	test_nested_additional_property_context();
	test_property_names_inside_logical_combinations();
	test_combination_failure_is_reported_even_if_reevaluation_passes();
	test_probe_evaluates_once();
	test_reentrant_validation_inside_probe();
	test_probe_state_after_unwinding_and_reentrant_handler();
	test_combination_default_values();
	test_any_of_collects_defaults_of_every_successful_branch();
	test_validation_error_is_a_value_type();
	test_is_valid();
	test_non_keyword_error_details();
	test_basic_error_handler();
	test_incomplete_error_handler();
	test_legacy_error_handler();
	return error_count;
}

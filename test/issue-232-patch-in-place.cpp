#include <chrono>
#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;
using namespace std::chrono;

void add_sub_schema(json &schema, int depth)
{
	schema["type"] = "object";
	schema["default"] = json::object();
	schema["properties"] = R"(
            {
                "array": {
                    "type": "array",
                    "default": [
                        {"name": "foo"}
                    ],
                    "items": {
                        "required": ["name"],
                        "oneOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "name": {
                                        "enum": "foo"
                                    },
                                    "code": {
                                        "const": 1,
                                        "default": 1
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "name": {
                                        "enum": "bar"
                                    },
                                    "code": {
                                        "const": 2,
                                        "default": 2
                                    }
                                }
                            }
                        ]
                    }
                }
            }
        )"_json;
	if (--depth >= 0) {
		json &properties = schema.at("/properties/array/items/oneOf/0/properties"_json_pointer);
		properties["sub"] = json::object();
		add_sub_schema(properties.at("sub"), depth);
	}
}

void add_sub_data(json &data, int depth)
{
	data["array"] = R"(
        [
            {
                "name": "foo",
                "code": 1
            }
        ]
    )"_json;
	if (--depth >= 0) {
		json &item = data.at("/array/0"_json_pointer);
		item["sub"] = json::object();
		add_sub_schema(item.at("sub"), depth);
	}
}

static const int DEPTH = 100;
static const json get_schema()
{
	static json schema;
	if (schema.empty()) {
		schema = R"(
            {
                "$schema": "http://json-schema.org/draft-07/schema#"
            }
        )"_json;
		add_sub_schema(schema, DEPTH);
	}

	return schema;
}

static void loader(const json_uri &uri, json &schema)
{
	schema = get_schema();
}

int main(void)
{
	json_validator validator(loader);

	validator.set_root_schema(get_schema());

	json data = json::object();
	json expected(data);
	add_sub_data(expected, DEPTH);

	auto start = high_resolution_clock::now();
#if 1
	validator.validate_inplace(data);
#else
	size_t count = 0;
	while (true) { // https://github.com/pboettch/json-schema-validator/issues/206#issuecomment-1173404152
		json patch = validator.validate(data);
		if (patch.empty()) {
			break;
		}
		++count;
		data.patch_inplace(patch);
	}
	std::cout << "Number of iterations: " << count << std::endl;
#endif
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	if (duration.count() > 300000) {
		std::cerr << "To long duration: " << duration.count() << " us" << std::endl;
		return 1;
	}

	auto diff = json::diff( //
	    data,               //
	    expected            //
	);
	if (diff.empty()) {
		std::cerr << "Unexpected data: '" << data.dump() << "' instead of expected '" << expected.dump() << "' differences are:" << diff.dump() << std::endl;
		return 1;
	}

	return 0;
}

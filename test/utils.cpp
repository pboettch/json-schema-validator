#include <fstream>

#include "utils.h"

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::default_string_format_check;
using nlohmann::json_schema::json_validator;

static void loader(const json_uri &uri, json &schema)
{
	std::string filename = "./" + uri.path();
	std::ifstream fstream(filename);
	if (!fstream.good())
		throw std::invalid_argument("could not open " + uri.url() + " tried with " + filename);
	try {
		fstream >> schema;
	} catch (const std::exception &e) {
		throw e;
	}
}

JsonValidateFixture::JsonValidateFixture()
    : validator(loader, default_string_format_check) {}
json JsonValidateFixture::validate(const std::filesystem::path &schema_path, const std::filesystem::path &instance_path)
{
	json schema;
	std::ifstream fstream{absolute(schema_path).string()};
	fstream >> schema;
	return validate(schema, instance_path);
}
json JsonValidateFixture::validate(const json &schema, const json &instance)
{
	validator.set_root_schema(schema);
	return validator.validate(instance);
}
json JsonValidateFixture::validate(const json &schema, const std::filesystem::path &instance_path)
{
	json instance;
	std::ifstream fstream{absolute(instance_path).string()};
	fstream >> instance;
	return validate(schema, instance);
}
json JsonValidateFixture::validate(std::string_view schema_path, std::string_view instance_path)
{
	auto path = std::filesystem::current_path() / schema_path;
	return validate(path, instance_path);
}
json JsonValidateFixture::validate(const std::filesystem::path &schema_path, std::string_view instance_path)
{
	auto path = std::filesystem::current_path() / instance_path;
	return validate(schema_path, path);
}
json JsonValidateFixture::validate(const std::filesystem::path &schema_path, const json &instance)
{
	json schema;
	std::ifstream fstream{absolute(schema_path).string()};
	fstream >> schema;
	return validate(schema, instance);
}

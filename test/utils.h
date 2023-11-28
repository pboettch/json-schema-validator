#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json-schema.hpp>

class JsonValidateFixture
{
	nlohmann::json_schema::json_validator validator;

public:
	JsonValidateFixture();
	nlohmann::json validate(std::string_view schema_path, std::string_view instance_path);
	nlohmann::json validate(const std::filesystem::path &schema_path, std::string_view instance_path);
	nlohmann::json validate(const std::filesystem::path &schema_path, const std::filesystem::path &instance_path);
	nlohmann::json validate(const std::filesystem::path &schema_path, const nlohmann::json &instance);
	nlohmann::json validate(const nlohmann::json &schema, const std::filesystem::path &instance_path);
	nlohmann::json validate(const nlohmann::json &schema, const nlohmann::json &instance);
};

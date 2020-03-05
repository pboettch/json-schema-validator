#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

static const json address_schema = R"(
{
  "type": "object",
  "properties": {
    "street_address": {
      "type": "string"
    },
    "country": {
      "enum": ["United States of America", "Canada"],
      "default": "Canada"
    }
  },
  "if": {
    "properties": { "country": { "const": "United States of America" } }
  },
  "then": {
    "properties": { "postal_code": { "pattern": "[0-9]{5}(-[0-9]{4})?" } }
  },
  "else": {
    "properties": { "postal_code": { "pattern": "[A-Z][0-9][A-Z] [0-9][A-Z][0-9]" } }
  }
})"_json;

class expected_error_handler : public nlohmann::json_schema::basic_error_handler
{
	void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer, const json &instance,
	           const std::string &message) override
	{
		hit_ = true;
	}

	bool hit_{false};

public:
	bool isHit() const { return hit_; }
};

class custom_error_handler : public nlohmann::json_schema::basic_error_handler
{
	void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer, const json &instance,
	           const std::string &message) override
	{
		nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
		std::cerr << "ERROR: '" << pointer << "' - '" << instance << "': " << message << "\n";
		hit_ = true;
	}

	bool hit_{false};

public:
	bool isHit() const { return hit_; }
};

int main(void)
{
	json_validator validator{};
	validator.set_root_schema(address_schema);

	{
		custom_error_handler customErr{};
		const auto good_withCanada = R"({
      "street_address": "24 Sussex Drive",
      "country": "Canada",
      "postal_code": "K1M 1M4"
    })"_json;
		validator.validate(good_withCanada, customErr);
		if (customErr.isHit()) {
			return 1;
		}
	}

	{
		custom_error_handler customErr{};
		const auto good_withDefaultCanada = R"({
      "street_address": "24 Sussex Drive",
      "postal_code": "K1M 1M4"
    })"_json;
		validator.validate(good_withDefaultCanada, customErr);
		if (customErr.isHit()) {
			return 1;
		}
	}

	{
		custom_error_handler customErr{};
		const auto good_Usa = R"({
      "street_address": "1600 Pennsylvania Avenue NW",
      "country": "United States of America",
      "postal_code": "20500"
    })"_json;
		validator.validate(good_Usa, customErr);
		if (customErr.isHit()) {
			return 1;
		}
	}

	{
		expected_error_handler expErr{};
		const auto bad_Canada = R"({
      "street_address": "24 Sussex Drive",
      "country": "Canada",
      "postal_code": "10000"
    })"_json;

		validator.validate(bad_Canada, expErr);
		if (!expErr.isHit()) {
			std::cerr << "Should have failed validating postal code" << std::endl;
			return 1;
		}
	}

	return 0;
}

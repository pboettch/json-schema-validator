#include <iostream>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static const json root_default = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "properties": {
        "width": {
            "type": "integer"
        }
    },
    "default": {
        "width": 42
    }
})"_json;

int main(void)
{
	json_validator validator{};

	validator.set_root_schema(root_default);

	{
		json nul_json;
		if (!nul_json.is_null()) {
			return 1;
		}

		const auto default_patch = validator.validate(nul_json);

		if (default_patch.is_null()) {
			std::cerr << "Patch is null but should contain operation to add defaults to root" << std::endl;
			return 1;
		}

		const auto actual = nul_json.patch(default_patch);
		const auto expected = R"({"width": 42})"_json;
		if (actual != expected) {
			std::cerr << "Patch of defaults is wrong for root schema: '" << actual.dump() << "' instead of expected '" << expected.dump() << "'" << std::endl;
		}
	}

	return 0;
}

#include <filesystem>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "utils.h"

using namespace std::literals;
using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::MessageMatches;

static const auto instance = R"(
[
    {
        "name":"player",
        "renderable": {
            "fg":"White"
        }
    }
]
)"_json;

static const auto expected_patch = R"(
[{"op":"add","path":"/0/renderable/bg","value":"Black"}]
)"_json;

TEST_CASE_METHOD(JsonValidateFixture, "issue-93")
{
	// Change the working directory to the issue path
	auto path = std::filesystem::path(__FILE__).parent_path();
	nlohmann::json patch;
	REQUIRE_NOTHROW(std::filesystem::current_path(path));
	auto schema_path = path / "blueprints.schema.json";
	REQUIRE_NOTHROW(patch = validate(schema_path, instance));
	REQUIRE(patch == expected_patch);
}

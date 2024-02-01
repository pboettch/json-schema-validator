#include <filesystem>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "utils.h"

using namespace std::literals;
using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::MessageMatches;

TEST_CASE_METHOD(JsonValidateFixture, "issue-96")
{
	// Change the working directory to the issue path
	auto path = std::filesystem::path(__FILE__).parent_path();
	REQUIRE_NOTHROW(std::filesystem::current_path(path));
	REQUIRE_THROWS_MATCHES(validate("schema.json"sv, "instance.json"sv), std::invalid_argument, MessageMatches(ContainsSubstring("instance exceeds maximum")));
}

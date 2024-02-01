#include <filesystem>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "utils.h"

using namespace std::literals;

TEST_CASE_METHOD(JsonValidateFixture, "issue-54")
{
	// Change the working directory to the issue path
	auto path = std::filesystem::path(__FILE__).parent_path();
	REQUIRE_NOTHROW(std::filesystem::current_path(path));
	REQUIRE_NOTHROW(validate("schema.json"sv, "instance.json"sv));
}

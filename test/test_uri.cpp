/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_uri;

namespace nlohmann
{
bool operator==(const json_uri &a, const std::string &b)
{
	return a.to_string() == b;
}
} // namespace nlohmann

void paths(json_uri start,
           const std::string &full,
           const std::string &full_path,
           const std::string &no_path)
{
	CHECK(start == full + " # ");

	auto a = start.derive("other.json");
	CHECK(a == full_path + "/other.json # ");

	auto b = a.derive("base.json");
	CHECK(b == full_path + "/base.json # ");

	auto c = b.derive("subdir/base.json");
	CHECK(c == full_path + "/subdir/base.json # ");

	auto d = c.derive("subdir2/base.json");
	CHECK(d == full_path + "/subdir/subdir2/base.json # ");

	auto e = c.derive("/subdir2/base.json");
	CHECK(e == no_path + "/subdir2/base.json # ");

	auto f = c.derive("new.json");
	CHECK(f == full_path + "/subdir/new.json # ");

	auto g = c.derive("/new.json");
	CHECK(g == no_path + "/new.json # ");
}

static void pointer_plain_name(json_uri start,
                               const std::string &full,
                               const std::string &full_path,
                               const std::string &no_path)
{
	auto a = start.derive("#/json/path");
	CHECK(a == full + " # /json/path");

	a = start.derive("#/json/special_%22");
	CHECK(a == full + " # /json/special_\"");

	a = a.derive("#foo");
	CHECK(a == full + " # foo");

	a = a.derive("#foo/looks_like_json/poiner/but/isnt");
	CHECK(a == full + " # foo/looks_like_json/poiner/but/isnt");
	CHECK(a.identifier() == "foo/looks_like_json/poiner/but/isnt");
	CHECK(a.pointer().to_string().empty());

	a = a.derive("#/looks_like_json/poiner/and/it/is");
	CHECK(a == full + " # /looks_like_json/poiner/and/it/is");
	CHECK(a.pointer().to_string() == "/looks_like_json/poiner/and/it/is");
	CHECK(a.identifier().empty());
}

TEST_CASE("uri")
{
	json_uri empty("");
	paths(empty,
	      "",
	      "",
	      "");

	json_uri http("http://json-schema.org/draft-07/schema#");
	paths(http,
	      "http://json-schema.org/draft-07/schema",
	      "http://json-schema.org/draft-07",
	      "http://json-schema.org");

	pointer_plain_name(http,
	                   "http://json-schema.org/draft-07/schema",
	                   "http://json-schema.org/draft-07",
	                   "http://json-schema.org");
}

/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <cstdlib>
#include <nlohmann/json-schema.hpp>

#include <iostream>
#include <sstream>

using nlohmann::json;
using nlohmann::json_uri;

static int errors;

static void EXPECT_EQ(const std::string &a, const std::string &b)
{
	if (a != b) {
		std::cerr << "Failed: '" << a << "' != '" << b << "'\n";
		errors++;
	}
}

static void EXPECT_EQ(const nlohmann::json_uri &a, const std::string &b)
{
	EXPECT_EQ(a.to_string(), b);
}

static void paths(json_uri start,
                  const std::string &full,
                  const std::string &full_path,
                  const std::string &no_path)
{
	EXPECT_EQ(start, full + " # ");

	auto a = start.derive("other.json");
	EXPECT_EQ(a, full_path + "/other.json # ");

	auto b = a.derive("base.json");
	EXPECT_EQ(b, full_path + "/base.json # ");

	auto c = b.derive("subdir/base.json");
	EXPECT_EQ(c, full_path + "/subdir/base.json # ");

	auto d = c.derive("subdir2/base.json");
	EXPECT_EQ(d, full_path + "/subdir/subdir2/base.json # ");

	auto e = c.derive("/subdir2/base.json");
	EXPECT_EQ(e, no_path + "/subdir2/base.json # ");

	auto f = c.derive("new.json");
	EXPECT_EQ(f, full_path + "/subdir/new.json # ");

	auto g = c.derive("/new.json");
	EXPECT_EQ(g, no_path + "/new.json # ");
}

static void pointer_plain_name(json_uri start,
                               const std::string &full,
                               const std::string & /*full_path*/,
                               const std::string & /*no_path*/)
{
	auto a = start.derive("#/json/path");
	EXPECT_EQ(a, full + " # /json/path");

	a = start.derive("#/json/special_%22");
	EXPECT_EQ(a, full + " # /json/special_\"");

	a = a.derive("#foo");
	EXPECT_EQ(a, full + " # foo");

	a = a.derive("#foo/looks_like_json/poiner/but/isnt");
	EXPECT_EQ(a, full + " # foo/looks_like_json/poiner/but/isnt");
	EXPECT_EQ(a.identifier(), "foo/looks_like_json/poiner/but/isnt");
	EXPECT_EQ(a.pointer().to_string(), "");

	a = a.derive("#/looks_like_json/poiner/and/it/is");
	EXPECT_EQ(a, full + " # /looks_like_json/poiner/and/it/is");
	EXPECT_EQ(a.pointer().to_string(), "/looks_like_json/poiner/and/it/is");
	EXPECT_EQ(a.identifier(), "");
}

// %-escapes in the fragment are decoded once, an incomplete or invalid escape is kept
static void percent_decoding()
{
	json_uri base("http://example.com/schema.json");

	EXPECT_EQ(base.derive("#/a%20b"), "http://example.com/schema.json # /a b");
	EXPECT_EQ(base.derive("#/%41%42"), "http://example.com/schema.json # /AB");
	EXPECT_EQ(base.derive("#/a%2525"), "http://example.com/schema.json # /a%25");
	EXPECT_EQ(base.derive("#/a%zz"), "http://example.com/schema.json # /a%zz");
	EXPECT_EQ(base.derive("#/a%2"), "http://example.com/schema.json # /a%2");
	EXPECT_EQ(base.derive("#/a%"), "http://example.com/schema.json # /a%");
	EXPECT_EQ(base.derive("#%a"), "http://example.com/schema.json # %a"); // looped forever
	EXPECT_EQ(base.derive("#%"), "http://example.com/schema.json # %");
}

// "." and ".." segments are removed, so that different paths to the same document result in the same URI
static void dot_segments()
{
	json_uri v2("http://acme.com/schemas/v2/v2.json");

	EXPECT_EQ(v2.derive("./../v1/v1.json"), "http://acme.com/schemas/v1/v1.json # ");
	EXPECT_EQ(v2.derive("../common.json#/definitions/attr"), "http://acme.com/schemas/common.json # /definitions/attr");
	EXPECT_EQ(v2.derive("sub/./x/../v1.json"), "http://acme.com/schemas/v2/sub/v1.json # ");
	EXPECT_EQ(v2.derive("/schemas/./v2/../v1/v1.json"), "http://acme.com/schemas/v1/v1.json # ");
	EXPECT_EQ(v2.derive("sub/."), "http://acme.com/schemas/v2/sub/ # ");
	EXPECT_EQ(v2.derive("sub/x/.."), "http://acme.com/schemas/v2/sub/ # ");
	EXPECT_EQ(v2.derive("sub/x/../"), "http://acme.com/schemas/v2/sub/ # ");

	if (!(v2.derive("./../v1/v1.json") == json_uri("http://acme.com/schemas/v1/v1.json"))) {
		std::cerr << "Failed: URIs with and without dot-segments are not equal\n";
		errors++;
	}

	// a ".." going above the beginning of the path is kept (e.g. relative to the current directory)
	json_uri empty("");
	EXPECT_EQ(empty.derive("../common.json"), "/../common.json # ");
	EXPECT_EQ(empty.derive("../sub/../common.json"), "/../common.json # ");
}

// a URL without path, output, accessors, plain-name fragments and URNs
static void misc()
{
	json_uri host("http://example.com");
	EXPECT_EQ(host, "http://example.com # ");
	EXPECT_EQ(host.url(), "http://example.com");
	EXPECT_EQ(host.derive("schema.json"), "http://example.com/schema.json # ");

	std::ostringstream os;
	os << json_uri("http://example.com/schema.json#/definitions/a");
	EXPECT_EQ(os.str(), "http://example.com/schema.json # /definitions/a");

	// a plain-name fragment has no pointer to append to
	json_uri plain("http://example.com/schema.json#foo");
	EXPECT_EQ(plain.append("bar"), "http://example.com/schema.json # foo");

	// an URN has no path
	json_uri urn("urn:example:schema");
	try {
		urn.derive("other.json");
		std::cerr << "Failed: a path was added to an URN\n";
		errors++;
	} catch (const std::invalid_argument &) {
	}
}

int main(void)
{
	percent_decoding();
	dot_segments();

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

	misc();

	return errors;
}

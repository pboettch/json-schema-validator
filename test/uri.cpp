/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <cstdlib>
#include <json-schema.hpp>

#include <iostream>

using nlohmann::json;
using nlohmann::json_uri;

static int errors;

#define EXPECT_EQ(a, b)                                              \
	do {                                                             \
		if (a.to_string() != b) {                                    \
			std::cerr << "Failed: '" << a << "' != '" << b << "'\n"; \
			errors++;                                                \
		}                                                            \
	} while (0)

// test-schema taken from spec with modifications
auto schema = R"(
{
	"$id": "http://example.com/root.json",
	"definitions": {
		"A": { "$id": "#foo" },
		"B": {
			"$id": "other.json",
			"definitions": {
				"X": { "$id": "#bar" },
				"Y": { "$id": "t/inner.json" }
			}
		},
		"C": {
			"$id": "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f",
			"definitions": {
				"Z": { "$id": "#bar" },
				"9": { "$id": "http://example.com/drole.json" }
			}
		}
	}
}
)"_json;

// resolve all refs
class store
{
public:
	std::vector<json> schemas_;
	std::map<nlohmann::json_uri, const json *> schema_store_;

public:
	void insert_schema(json &s, std::vector<nlohmann::json_uri> base_uris)
	{
		auto id = s.find("$id");
		if (id != s.end())
			base_uris.push_back(base_uris.back().derive(id.value()));

		for (auto &u : base_uris)
			schema_store_[u] = &s;

		for (auto i = s.begin();
		     i != s.end();
		     ++i) {

			switch (i.value().type()) {
			case json::value_t::object: { // child is object, thus a schema
				std::vector<nlohmann::json_uri> subschema_uri = base_uris;

				for (auto &ss : subschema_uri)
					ss = ss.append(nlohmann::json_uri::escape(i.key()));

				insert_schema(i.value(), subschema_uri);
			} break;

			case json::value_t::string:
				// this schema is a reference
				if (i.key() == "$ref") {
					auto id = base_uris.back().derive(i.value());
					i.value() = id.to_string();
				}

				break;

			default:
				break;
			}
		}
	}
};

//static void store_test(void)
//{
//	store  s;
//
//	s.insert_schema(schema, {{""}});
//
//	for (auto &i : s.schema_store_) {
//		std::cerr << i.first << " " << *i.second << "\n";
//	}
//}

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

int main(void)
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

	// plain name fragments instead of JSON-pointers, are not supported, yet
	//store_test();

	return errors;
}

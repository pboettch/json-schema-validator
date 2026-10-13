/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include <nlohmann/json-schema.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <vector>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static void loader(const json_uri &uri, json &schema)
{
	if (uri.location() == "http://json-schema.org/draft-07/schema") {
		schema = nlohmann::json_schema::draft7_schema_builtin;
		return;
	}

	std::string fn = JSON_SCHEMA_TEST_SUITE_PATH;
	fn += "/remotes";
	fn += uri.path();
	std::cerr << fn << "\n";

	std::ifstream s(fn);
	if (!s.good())
		throw std::invalid_argument("could not open " + uri.url() + " for schema loading\n");

	try {
		s >> schema;
	} catch (std::exception &e) {
		throw e;
	}
}

// from here
// https://stackoverflow.com/a/34571089/880584
static std::string base64_decode(const std::string &in)
{
	std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++)
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	unsigned val = 0;
	int valb = -8;
	for (uint8_t c : in) {
		if (c == '=')
			break;

		if (T[c] == -1) {
			throw std::invalid_argument("base64-decode: unexpected character in encode string: '" + std::string(1, c) + "'");
		}
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}

static void content(const std::string &contentEncoding, const std::string &contentMediaType, const json &instance)
{
	std::string content = instance.get<std::string>();

	if (contentEncoding == "base64")
		content = base64_decode(instance.get<std::string>());
	else if (contentEncoding != "")
		throw std::invalid_argument("unable to check for contentEncoding '" + contentEncoding + "'");

	if (contentMediaType == "application/json")
		auto dummy = json::parse(content); // throws if conversion fails
	else if (contentMediaType != "")
		throw std::invalid_argument("unable to check for contentMediaType '" + contentMediaType + "'");
}

// test cases of a test file which are known to fail, read from known-failures.json
struct known_failure {
	std::string group;
	std::set<std::string> tests; // empty: all test cases of the group
	std::string reason;
	size_t matched;
};

static std::vector<known_failure> read_known_failures(const std::string &filename, const std::string &test_file)
{
	std::ifstream s(filename);
	if (!s.good())
		throw std::invalid_argument("could not open " + filename);

	json list;
	s >> list;

	std::vector<known_failure> known_failures;
	for (auto &entry : list) {
		if (entry.at("file").get<std::string>() != test_file)
			continue;

		known_failure known;
		known.group = entry.at("group").get<std::string>();
		if (entry.count("tests"))
			for (auto &test : entry["tests"])
				known.tests.insert(test.get<std::string>());
		known.reason = entry.at("reason").get<std::string>();
		known.matched = 0;
		known_failures.push_back(known);
	}

	return known_failures;
}

static known_failure *find_known_failure(std::vector<known_failure> &known_failures, const std::string &group, const std::string &test)
{
	for (auto &known : known_failures)
		if (known.group == group && (known.tests.empty() || known.tests.count(test))) {
			known.matched++;
			return &known;
		}

	return nullptr;
}

int main(int argc, char *argv[])
{
	// optional: a list of known failures and the name of the test file in it
	std::vector<known_failure> known_failures;
	if (argc == 3) {
		try {
			known_failures = read_known_failures(argv[1], argv[2]);
		} catch (std::exception &e) {
			std::cout << "could not read known failures: " << e.what() << "\n";
			return EXIT_FAILURE;
		}
	} else if (argc != 1) {
		std::cerr << "usage: " << argv[0] << " [<known-failures.json> <test-file>] < test-file\n";
		return EXIT_FAILURE;
	}

	json validation; // a validation case following the JSON-test-suite-schema

	try {
		std::cin >> validation;
	} catch (std::exception &e) {
		std::cout << e.what() << "\n";
		return EXIT_FAILURE;
	}

	size_t total_failed = 0,
	       total_known = 0,
	       total = 0;

	for (auto &test_group : validation) {
		size_t group_failed = 0,
		       group_known = 0,
		       group_total = 0;

		const auto group = test_group["description"].get<std::string>();
		std::cout << "Testing Group " << test_group["description"] << "\n";

		const auto &schema = test_group["schema"];

		json_validator validator(loader,
		                         nlohmann::json_schema::default_string_format_check,
		                         content);

		// a schema which cannot be used fails all test cases of its group
		bool schema_failed = false;
		try {
			validator.set_root_schema(schema);
		} catch (const std::exception &e) {
			schema_failed = true;
			std::cout << "  Schema Exception: " << e.what() << "\n";
		}

		for (auto &test_case : test_group["tests"]) {
			const auto description = test_case["description"].get<std::string>();
			const bool expected = test_case["valid"].get<bool>();
			std::cout << "  Testing Case " << test_case["description"] << "\n";

			bool valid = true;

			if (schema_failed)
				valid = !expected;
			else
				try {
					validator.validate(test_case["data"]);
				} catch (const std::out_of_range &e) {
					valid = false;
					std::cout << "    Test Case Exception (out of range): " << e.what() << "\n";

				} catch (const std::invalid_argument &e) {
					valid = false;
					std::cout << "    Test Case Exception (invalid argument): " << e.what() << "\n";

				} catch (const std::logic_error &e) {
					valid = !expected; /* force test-case failure */
					std::cout << "    Not yet implemented: " << e.what() << "\n";
				}

			const known_failure *known = find_known_failure(known_failures, group, description);

			if (valid == expected && !known)
				std::cout << "      --> Test Case exited with " << valid << " as expected.\n";
			else if (valid == expected) {
				group_failed++;
				std::cout << "      --> Test Case exited with " << valid << " as expected, but is listed as known failure. Remove it from the known failures.\n";
			} else if (known) {
				group_known++;
				std::cout << "      --> Test Case exited with " << valid << ", known failure: " << known->reason << "\n";
			} else {
				group_failed++;
				std::cout << "      --> Test Case exited with " << valid << " NOT expected.\n";
			}
			group_total++;
			std::cout << "\n";
		}
		total_failed += group_failed;
		total_known += group_known;
		total += group_total;
		std::cout << "Group RESULT: " << test_group["description"] << " "
		          << (group_total - group_failed - group_known) << " of " << group_total
		          << " have succeeded - " << group_failed << " failed - " << group_known << " known failures\n";
		std::cout << "-------------\n";
	}

	// known failures which do not match the test file anymore
	size_t stale = 0;
	for (auto &known : known_failures)
		if (known.matched == 0 || (!known.tests.empty() && known.matched != known.tests.size())) {
			stale++;
			std::cout << "Known failure of group \"" << known.group << "\" does not match the test cases of this file (" << known.matched << " matched). Update the known failures.\n";
		}

	std::cout << "Total RESULT: " << (total - total_failed - total_known) << " of " << total << " have succeeded - " << total_failed << " failed - " << total_known << " known failures\n";

	return total_failed == 0 && stale == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

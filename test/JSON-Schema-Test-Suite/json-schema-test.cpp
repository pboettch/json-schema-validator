/*
 * JSON schema validator for JSON for modern C++
 *
 * Copyright (c) 2016-2019 Patrick Boettcher <p@yai.se>.
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include "json-schema.hpp"

#include <fstream>
#include <iostream>
#include <regex>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

static void format_check(const std::string &format, const std::string &value)
{
	if (format == "hostname") {
		// from http://stackoverflow.com/questions/106179/regular-expression-to-match-dns-hostname-or-ip-address
		std::regex re(R"(^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$)");
		if (!std::regex_match(value, re))
			throw std::invalid_argument(value + " is not a valid hostname.");

	} else if (format == "ipv4") {
		std::regex re(R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)");
		if (!std::regex_match(value, re))
			throw std::invalid_argument(value + " is not a IPv4-address.");

	} else if (format == "ipv6") {
		std::regex re(R"((([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])))");
		if (!std::regex_match(value, re))
			throw std::invalid_argument(value + " is not a IPv6-address.");

	} else if (format == "regex") {
		try {
			std::regex re(value, std::regex::ECMAScript);
		} catch (std::exception &e) {
			throw e;
		}
	} else
		throw std::logic_error("don't know how to validate " + format);
}

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

	std::fstream s(fn.c_str());
	if (!s.good())
		throw std::invalid_argument("could not open " + uri.url() + " for schema loading\n");

	try {
		s >> schema;
	} catch (std::exception &e) {
		throw e;
	}
}

int main(void)
{
	json validation; // a validation case following the JSON-test-suite-schema

	try {
		std::cin >> validation;
	} catch (std::exception &e) {
		std::cout << e.what() << "\n";
		return EXIT_FAILURE;
	}

	size_t total_failed = 0,
	       total = 0;

	for (auto &test_group : validation) {
		size_t group_failed = 0,
		       group_total = 0;

		std::cout << "Testing Group " << test_group["description"] << "\n";

		const auto &schema = test_group["schema"];

		json_validator validator(loader, format_check);

		validator.set_root_schema(schema);

		for (auto &test_case : test_group["tests"]) {
			std::cout << "  Testing Case " << test_case["description"] << "\n";

			bool valid = true;

			try {
				validator.validate(test_case["data"]);
			} catch (const std::out_of_range &e) {
				valid = false;
				std::cout << "    Test Case Exception (out of range): " << e.what() << "\n";

			} catch (const std::invalid_argument &e) {
				valid = false;
				std::cout << "    Test Case Exception (invalid argument): " << e.what() << "\n";

			} catch (const std::logic_error &e) {
				valid = !test_case["valid"]; /* force test-case failure */
				std::cout << "    Not yet implemented: " << e.what() << "\n";
			}

			if (valid == test_case["valid"])
				std::cout << "      --> Test Case exited with " << valid << " as expected.\n";
			else {
				group_failed++;
				std::cout << "      --> Test Case exited with " << valid << " NOT expected.\n";
			}
			group_total++;
			std::cout << "\n";
		}
		total_failed += group_failed;
		total += group_total;
		std::cout << "Group RESULT: " << test_group["description"] << " "
		          << (group_total - group_failed) << " of " << group_total
		          << " have succeeded - " << group_failed << " failed\n";
		std::cout << "-------------\n";
	}

	std::cout << "Total RESULT: " << (total - total_failed) << " of " << total << " have succeeded - " << total_failed << " failed\n";

	return total_failed;
}

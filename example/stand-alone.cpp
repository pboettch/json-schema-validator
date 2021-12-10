/*
 * JSON schema validator for JSON for modern C++
 * SPDX-License-Identifier: MIT
 */

#include <nlohmann/json-schema.hpp>

#include <fstream>
#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::default_string_format_check;
using nlohmann::json_schema::json_validator;

static int usage(const char *name)
{
	std::cerr << "Usage: " << name << " <schema> <document>\n";
	return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		return usage(argv[0]);

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		ifs.open(argv[1]);
		json schema = json::parse(ifs);

		json_validator validator(nullptr, default_string_format_check);
		validator.set_root_schema(schema);

		ifs.open(argv[2]);
		json document = json::parse(ifs);

		validator.validate(document);
	}
	catch(const std::ifstream::failure& e) {
		std::cerr << "Exception opening/reading file" << std::endl;
		return EXIT_FAILURE;
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown error" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "document is valid !" << std::endl;
	return EXIT_SUCCESS;
}

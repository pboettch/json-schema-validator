#include "json-schema-validator.hpp"

#include <fstream>

#include <cstdlib>

using nlohmann::json;
using nlohmann::json_validator;

static void usage(const char *name)
{
	std::cerr << "Usage: " << name << " <json-document> < <schema>\n";
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		usage(argv[0]);

	std::fstream f(argv[1]);
	if (!f.good()) {
		std::cerr << "could not open " << argv[1] << " for reading\n";
		usage(argv[0]);
	}

	json schema;

	try {
		f >> schema;
	} catch (std::exception &e) {
		std::cerr << e.what() << " at " << f.tellp() << "\n";
		return EXIT_FAILURE;
	}

	json document;

	try {
		std::cin >> document;
	} catch (std::exception &e) {
		std::cerr << e.what() << " at " << f.tellp() << "\n";
		return EXIT_FAILURE;
	}

	json_validator validator;

	try {
		validator.validate(document, schema);
	} catch (std::exception &e) {
		std::cerr << "schema validation failed\n";
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	std::cerr << std::setw(2) << document << "\n";

	return EXIT_SUCCESS;
}

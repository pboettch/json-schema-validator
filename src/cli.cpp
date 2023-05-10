#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <fstream>

#include "nlohmann/json-schema.hpp"

using namespace nlohmann;
using namespace nlohmann::json_schema;

int main(int argc, char *argv[])
{
	std::ifstream schema_input;
	std::ifstream object_input;

	CLI::App app{"Json schema validator", "json-validator"};
	// TODO: Move to a generated header file
	app.set_version_flag("--version", "2.2.0");
	app.add_option("schema", schema_input, "JSON schema of the object")
	    ->check(CLI::ExistingFile);
	app.add_option("object", "JSON object to validate")
	    ->check(CLI::ExistingFile);

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	json schema{};
	json object{};
	if (!schema_input.good())
		throw std::invalid_argument("could not read schema");
	if (!object_input.good())
		throw std::invalid_argument("could not read object");
	schema_input >> schema;
	object_input >> object;
	return 0;
}

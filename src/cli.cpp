#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <fstream>

#include "nlohmann/json-schema.hpp"

using namespace nlohmann;
using namespace nlohmann::json_schema;

class main_cli : public CLI::App
{
	std::ifstream schema_input;
	std::ifstream object_input;
	// TODO: Export this as a built-in loader
	static void loader(const json_uri &uri, json &schema)
	{
		std::string filename = "./" + uri.path();
		std::ifstream lf(filename);
		if (!lf.good())
			throw std::invalid_argument("could not open " + uri.url() + " tried with " + filename);
		try {
			lf >> schema;
		} catch (const std::exception &e) {
			throw e;
		}
	}

public:
	json schema;
	json object;
	json_validator validator;
	main_cli()
	    : CLI::App{"Json schema validator", "json-validator"},
	      validator{loader, default_string_format_check}
	{
		// TODO: Move to a generated header file
		set_version_flag("--version", "2.2.0");
		add_option("schema", schema_input, "JSON schema of the object")
		    ->check(CLI::ExistingFile);
		add_option("object", object_input, "JSON object to validate")
		    ->check(CLI::ExistingFile);
	}
	void validate()
	{
		validator.set_root_schema(schema);
		validator.validate(object);
	}
};

int main(int argc, char *argv[])
{
	main_cli app{};

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}

	app.validate();

	return 0;
}

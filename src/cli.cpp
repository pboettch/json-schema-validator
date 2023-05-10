#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

int main(int argc, char *argv[])
{
	CLI::App app{"Json schema validator", "json-validator"};
	// TODO: Move to a generated header file
	app.set_version_flag("--version", "2.2.0");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	}
	return 0;
}

#include "json-schema-validator.hpp"

using nlohmann::json;
using nlohmann::json_validator;

int main(void)
{
	json validation;

	try {
		std::cin >> validation;
	} catch (std::exception &e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	json_validator validator;

	size_t failed = 0,
	       total = 0;

	for (auto &test_group : validation) {

		std::cerr << "Testing Group " << test_group["description"] << "\n";

		const auto &schema = test_group["schema"];

		for (auto &test_case : test_group["tests"]) {
			std::cerr << "  Testing Case " << test_case["description"] << "\n";

			bool valid = true;

			try {
				validator.validate(test_case["data"], schema);
			} catch (const std::out_of_range &e) {
				valid = false;
				std::cerr << "    Test Case Exception (out of range): " << e.what() << "\n";
			} catch (const std::invalid_argument &e) {
				valid = false;
				std::cerr << "    Test Case Exception (invalid argument): " << e.what() << "\n";
			} catch (const std::logic_error &e) {
				valid = !test_case["valid"]; /* force test-case failure */
				std::cerr << "    Not yet implemented: " << e.what() << "\n";
			}

			if (valid == test_case["valid"])
				std::cerr << "      --> Test Case exited with " << valid << " as expected.\n";
			else {
				failed++;
				std::cerr << "      --> Test Case exited with " << valid << " NOT expected.\n";
			}
			total++;
			std::cerr << "\n";
		}
		std::cerr << "-------------\n";
	}

	std::cerr << (total - failed) << " of " << total << " have succeeded - " << failed << " failed\n";

	return failed;
}

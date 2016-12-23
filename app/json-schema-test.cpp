#include "json-schema-validator.hpp"

using nlohmann::json;
using nlohmann::json_validator;

int main(void)
{
	json validation;

	try {
		std::cin >> validation;
	} catch (std::exception &e) {
		std::cout << e.what() << "\n";
		return EXIT_FAILURE;
	}

	json_validator validator;

	size_t total_failed = 0,
	       total = 0;

	for (auto &test_group : validation) {
		size_t group_failed = 0,
		       group_total = 0;

		std::cout << "Testing Group " << test_group["description"] << "\n";

		const auto &schema = test_group["schema"];

		for (auto &test_case : test_group["tests"]) {
			std::cout << "  Testing Case " << test_case["description"] << "\n";

			bool valid = true;

			try {
				validator.validate(test_case["data"], schema);
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

	std::cout << (total - total_failed) << " of " << total << " have succeeded - " << total_failed << " failed\n";

	return total_failed;
}

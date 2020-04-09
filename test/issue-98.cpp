#include <nlohmann/json-schema.hpp>

int main(void)
{
	nlohmann::json nlBase{{"$ref", "#/unknown/keywords"}};
	nlohmann::json_schema::json_validator validator;
	validator.set_root_schema(nlBase); // this line will log the caught exception

	return 0;
}

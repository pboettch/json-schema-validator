#include "json_patch.hpp"

namespace nlohmann
{

json_patch::json_patch()
    : j_{R"([])"_json} {}

json_patch::json_patch(json &&patch)
    : j_{std::move(patch)}
{
	validateJsonPatch(j_);
}

json_patch::json_patch(const json &patch)
    : j_{std::move(patch)}
{
	validateJsonPatch(j_);
}

json_patch &json_patch::add(std::string path, json value)
{
	j_.push_back(json{{"op", "add"}, {"path", std::move(path)}, {"value", std::move(value)}});
	return *this;
}

json_patch &json_patch::replace(std::string path, json value)
{
	j_.push_back(json{{"op", "replace"}, {"path", std::move(path)}, {"value", std::move(value)}});
	return *this;
}

json_patch &json_patch::remove(std::string path)
{
	j_.push_back(json{{"op", "remove"}, {"path", std::move(path)}});
	return *this;
}

void json_patch::validateJsonPatch(json const &patch)
{
	if (!patch.is_array()) {
		throw JsonPatchFormatException{"Json is not an array"};
	}

	for (auto const &op : patch) {
		if (!op.is_object()) {
			throw JsonPatchFormatException{"Each json patch entry needs to be an op object"};
		}

		if (!op.contains("op")) {
			throw JsonPatchFormatException{"Each json patch entry needs op element"};
		}

		const auto opType = op["op"].get<std::string>();
		if ((opType != "add") && (opType != "remove") && (opType != "replace")) {
			throw JsonPatchFormatException{std::string{"Operation "} + opType + std::string{"is invalid"}};
		}

		if (!op.contains("path")) {
			throw JsonPatchFormatException{"Each json patch entry needs path element"};
		}

		try {
			// try parse to path
			[[maybe_unused]] const auto p = json::json_pointer{op["path"].get<std::string>()};
		} catch (json::exception &e) {
			throw JsonPatchFormatException{e.what()};
		}

		if (opType != "remove") {
			if (!op.contains("value")) {
				throw JsonPatchFormatException{"Remove and replace needs value element"};
			}
		}
	}
}

} // namespace nlohmann

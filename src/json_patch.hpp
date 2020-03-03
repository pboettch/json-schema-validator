#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace nlohmann
{
class JsonPatchFormatException : public std::exception
{
public:
	explicit JsonPatchFormatException(std::string msg)
	    : ex_{std::move(msg)} {}

	inline const char *what() const noexcept override final { return ex_.c_str(); }

private:
	std::string ex_;
};

class json_patch
{
public:
	json_patch();
	json_patch(json &&patch);
	json_patch(const json &patch);

	json_patch &add(std::string path, json value);
	json_patch &replace(std::string path, json value);
	json_patch &remove(std::string path);

	operator json() const { return j_; }

private:
	json j_;

	static void validateJsonPatch(json const &patch);
};
} // namespace nlohmann

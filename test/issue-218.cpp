#include <nlohmann/json-schema.hpp>

#include <iostream>
#include <string>
#include <vector>

using nlohmann::json;
using nlohmann::json_uri;
using nlohmann::json_schema::json_validator;

namespace {

static int error_count;

template <typename T, typename R>
void EXPECT_EQ(T a, R b, std::string name="") {
  if (a != b) {
    if (name != "") {
      std::cerr << name << " - ";
    }
		std::cerr << "Failed: '" << a << "' != '" << b << "'\n";
		error_count++;
	}
}


static json schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Schedule",
    "type": "object",
    "properties": {
        "reply": {
            "description": "Send reply",
            "type": "boolean"
        },
        "schedule": {
            "description": "Array with schedule entries",
            "type": "array",
            "items": { "$ref": "#/definitions/schedule_entry" }
        }
    },
    "definitions": {
        "schedule_entry": {
            "type": "object",
            "required": [
                "id",
                "startdate",
                "cmd",
                "settings"
            ],
            "properties": {
                "id": {
                    "type": "string"
                },
                "startdate": {
                    "type": "number",
                    "minimum": 0
                },
                "cmd": {
                    "type": "string"
                },
                "settings": {
                    "type": "object"
                }
            }
        }
    }
}
)"_json;

json valid_schedule = R"(
{
    "reply": false, 
    "schedule": [
        {
            "id": "abc",
            "startdate": 1,
            "cmd": "foo",
            "settings": {}
        }
    ]
}
)"_json;

json valid_schedule_entry = R"(
{
    "id": "abc",
    "startdate": 1,
    "cmd": "foo",
    "settings": {}
}
)"_json;

json invalid_schedule_entry = R"(
{
    "id": 23,
    "startdate": 1,
    "cmd": "foo",
    "settings": {}
}
)"_json;

class store_err_handler : public nlohmann::json_schema::basic_error_handler {
public:
	void reset() override {
		nlohmann::json_schema::basic_error_handler::reset();
		failed_.clear();
	}
  std::size_t failed() const {
    return failed_.size();
  }
private:
	void error(const nlohmann::json::json_pointer &ptr, const json &instance, const std::string &message) override
	{
		nlohmann::json_schema::basic_error_handler::error(ptr, instance, message);
		std::cerr << "ERROR: '" << ptr << "' - '" << instance << "': " << message << "\n";
		failed_.push_back(ptr);
	}
	std::vector<nlohmann::json::json_pointer> failed_;
};

}  // namespace


int main() {
  json_validator validator(schema);
	store_err_handler err;

	validator.validate(valid_schedule, err); // OK
	EXPECT_EQ(err.failed(), 0, "valid object");
	err.reset();

	validator.validate(R"({ "foo": "bar" })"_json, err); 
	EXPECT_EQ(err.failed(), 1, "invalid object");
	err.reset();

	validator.validate(valid_schedule_entry, err, json_uri("#/definitions/schedule_entry"));
	EXPECT_EQ(err.failed(), 0, "valid sub-object");
	err.reset();
	
  validator.validate(invalid_schedule_entry, err, json_uri("#/definitions/schedule_entry"));
	EXPECT_EQ(err.failed(), 1, "invalid sub-object");
	err.reset();

	return error_count;
}

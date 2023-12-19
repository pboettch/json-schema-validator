#include "nlohmann/json-schema.hpp"
#include "nlohmann/json.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

//==============================================================================
// Test macros
//==============================================================================
#define LOG_ERROR(LOG_ERROR__ARGS)                                      \
    std::cerr << __FILE__ << ":" << __LINE__ << ": " << LOG_ERROR__ARGS << std::endl

#define EXPECT_THROW_WITH_MESSAGE(EXPRESSION, MESSAGE)                  \
    do                                                                  \
    {                                                                   \
        try                                                             \
        {                                                               \
            EXPRESSION;                                                 \
            LOG_ERROR("Expected exception not thrown with matching regex: \"" << MESSAGE << "\""); \
            ++g_error_count;                                            \
        } catch (const std::exception& error)                           \
        {                                                               \
            const std::regex error_re{ MESSAGE };                       \
            if (!std::regex_search(error.what(), error_re))             \
            {                                                           \
                LOG_ERROR("Expected exception with matching regex: \"" << MESSAGE << "\", but got this instead: " << error.what()); \
                ++g_error_count;                                        \
            }                                                           \
        }                                                               \
    } while (false)

#define ASSERT_OR_EXPECT_EQ(FIRST_THING, SECOND_THING, RETURN_IN_CASE_OF_ERROR) \
    do                                                                  \
    {                                                                   \
        if ((FIRST_THING) != (SECOND_THING))                            \
        {                                                               \
            LOG_ERROR("The two values of " << (FIRST_THING) << " (" #FIRST_THING << ") and " << (SECOND_THING) << " (" #SECOND_THING << ") should be equal"); \
            if (RETURN_IN_CASE_OF_ERROR)                                \
            {                                                           \
                return;                                                 \
            }                                                           \
        }                                                               \
    }                                                                   \
    while(false)

#define ASSERT_EQ(FIRST_THING, SECOND_THING) ASSERT_OR_EXPECT_EQ(FIRST_THING, SECOND_THING, true)
#define EXPECT_EQ(FIRST_THING, SECOND_THING) ASSERT_OR_EXPECT_EQ(FIRST_THING, SECOND_THING, true)

#define EXPECT_MATCH(STRING, REGEX)                                     \
    do                                                                  \
    {                                                                   \
        if (!std::regex_search((STRING), std::regex{ (REGEX) }))        \
        {                                                               \
            LOG_ERROR("String \"" << (STRING) << "\" doesn't match with regex: \"" << (REGEX) << "\""); \
            ++g_error_count;                                            \
        }                                                               \
    }                                                                   \
    while(false)

namespace
{

//==============================================================================
// Test environment
//==============================================================================
int g_error_count = 0;

//==============================================================================
// The schema used for testing
//==============================================================================
const std::string g_schema_template = R"(
{
    "properties": {
        "first": {
            "%COMBINATION_FIRST_LEVEL%": [
                {
                    "properties": {
                        "second": {
                            "%COMBINATION_SECOND_LEVEL%": [
                                {
                                    "minimum": 5,
                                    "type": "integer"
                                },
                                {
                                    "multipleOf": 2,
                                    "type": "integer"
                                }
                            ]
                        }
                    },
                    "type": "object"
                },
                {
                    "minimum": 20,
                    "type": "integer"
                },
                {
                    "minLength": 10,
                    "type": "string"
                }
            ]
        }
    },
    "type": "object"
}
)";

auto generateSchema(const std::string& first_combination, const std::string& second_combination) -> nlohmann::json
{
    static const std::regex first_replace_re{"%COMBINATION_FIRST_LEVEL%"};
    static const std::regex second_replace_re{"%COMBINATION_SECOND_LEVEL%"};

    std::string intermediate = std::regex_replace(g_schema_template, first_replace_re, first_combination);
    
    return nlohmann::json::parse(std::regex_replace(intermediate, second_replace_re, second_combination));
}

//==============================================================================
// Error handler to catch all the errors generated by the validator - also inside the combinations
//==============================================================================
class MyErrorHandler : public nlohmann::json_schema::error_handler
{
public:
    struct ErrorEntry
    {
        nlohmann::json::json_pointer ptr;
        nlohmann::json intance;
        std::string message;
    };

    using ErrorEntryList = std::vector<ErrorEntry>;

    auto getErrors() const -> const ErrorEntryList&
    {
        return m_error_list;
    }
    
private:
    auto error(const nlohmann::json::json_pointer& ptr, const nlohmann::json& instance, const std::string& message) -> void override
    {
        m_error_list.push_back(ErrorEntry{ptr, instance, message});
    }

    ErrorEntryList m_error_list;
};

//==============================================================================
// Error string helpers
//==============================================================================
auto operator<<(std::string first, const std::string& second) -> std::string
{
    first += ".*";
    first += second;
    return first;
}

auto rootError(const std::string& combination_type, std::size_t number_of_subschemas) -> std::string
{
    return "no subschema has succeeded, but one of them is required to validate. Type: " + combination_type + ", number of failed subschemas: " + std::to_string(number_of_subschemas);
}

auto combinationError(const std::string& combination_type, std::size_t test_case_number) -> std::string
{
    return "[combination: " + combination_type + " / case#" + std::to_string(test_case_number) + "]";
}

//==============================================================================
// Validator function - for simplicity
//==============================================================================
auto validate(const nlohmann::json& schema, const nlohmann::json& instance, nlohmann::json_schema::error_handler* error_handler = nullptr) -> void
{
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(schema);

    if (error_handler)
    {
        validator.validate(instance, *error_handler);
    }
    else
    {
        validator.validate(instance);
    }
}

//==============================================================================
// The test cases
//==============================================================================
auto simpleTest(const std::string& first_combination, const std::string& second_combination) -> void
{
    const nlohmann::json schema = generateSchema(first_combination, second_combination);
    EXPECT_THROW_WITH_MESSAGE(validate(schema, nlohmann::json{ { "first", { { "second", 1 } } } }), rootError(first_combination, 3));
    if (second_combination == "oneOf")
    {
        EXPECT_THROW_WITH_MESSAGE(validate(schema, nlohmann::json{ { "first", { { "second", 8 } } } }), rootError(first_combination, 3));
    }
    EXPECT_THROW_WITH_MESSAGE(validate(schema, nlohmann::json{ { "first", 10 } }), rootError(first_combination, 3));
    EXPECT_THROW_WITH_MESSAGE(validate(schema, nlohmann::json{ { "first", "short" } }), rootError(first_combination, 3));
}

auto verboseTest(const std::string& first_combination, const std::string& second_combination) -> void
{
    const nlohmann::json schema = generateSchema(first_combination, second_combination);

    {
        MyErrorHandler error_handler;
        validate(schema, nlohmann::json{ { "first", { { "second", 1 } } } }, &error_handler);

        const MyErrorHandler::ErrorEntryList& error_list = error_handler.getErrors();
        EXPECT_EQ(error_list.size(), 6);

        EXPECT_EQ(error_list[0].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[0].message, rootError(first_combination, 3));

        EXPECT_EQ(error_list[1].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[1].message, combinationError(first_combination, 0) << rootError(second_combination, 2));

        EXPECT_EQ(error_list[2].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[2].message, combinationError(first_combination, 0) << combinationError(second_combination, 0) << "instance is below minimum of 5");

        EXPECT_EQ(error_list[3].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[3].message, combinationError(first_combination, 0) << combinationError(second_combination, 1) << "instance is not a multiple of 2.0");

        EXPECT_EQ(error_list[4].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[4].message, combinationError(first_combination, 1) << "unexpected instance type");

        EXPECT_EQ(error_list[5].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[5].message, combinationError(first_combination, 2) << "unexpected instance type");
    }

    {
        MyErrorHandler error_handler;
        validate(schema, nlohmann::json{ { "first", { { "second", "not-an-integer" } } } }, &error_handler);

        const MyErrorHandler::ErrorEntryList& error_list = error_handler.getErrors();
        EXPECT_EQ(error_list.size(), 6);

        EXPECT_EQ(error_list[0].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[0].message, rootError(first_combination, 3));

        EXPECT_EQ(error_list[1].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[1].message, combinationError(first_combination, 0) << rootError(second_combination, 2));

        EXPECT_EQ(error_list[2].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[2].message, combinationError(first_combination, 0) << combinationError(second_combination, 0) << "unexpected instance type");

        EXPECT_EQ(error_list[3].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[3].message, combinationError(first_combination, 0) << combinationError(second_combination, 1) << "unexpected instance type");

        EXPECT_EQ(error_list[4].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[4].message, combinationError(first_combination, 1) << "unexpected instance type");

        EXPECT_EQ(error_list[5].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[5].message, combinationError(first_combination, 2) << "unexpected instance type");
    }

    if (second_combination == "oneOf")
    {
        MyErrorHandler error_handler;
        validate(schema, nlohmann::json{ { "first", { { "second", 8 } } } }, &error_handler);

        const MyErrorHandler::ErrorEntryList& error_list = error_handler.getErrors();
        EXPECT_EQ(error_list.size(), 4);

        EXPECT_EQ(error_list[0].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[0].message, rootError(first_combination, 3));

        EXPECT_EQ(error_list[1].ptr, nlohmann::json::json_pointer{ "/first/second" });
        EXPECT_MATCH(error_list[1].message, combinationError(first_combination, 0) << "more than one subschema has succeeded, but exactly one of them is required to validate");

        EXPECT_EQ(error_list[2].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[2].message, combinationError(first_combination, 1) << "unexpected instance type");

        EXPECT_EQ(error_list[3].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[3].message, combinationError(first_combination, 2) << "unexpected instance type");
    }

    {
        MyErrorHandler error_handler;
        validate(schema, nlohmann::json{ { "first", 10 } }, &error_handler);

        const MyErrorHandler::ErrorEntryList& error_list = error_handler.getErrors();
        EXPECT_EQ(error_list.size(), 4);

        EXPECT_EQ(error_list[0].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[0].message, rootError(first_combination, 3));

        EXPECT_EQ(error_list[1].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[1].message, combinationError(first_combination, 0) << "unexpected instance type");

        EXPECT_EQ(error_list[2].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[2].message, combinationError(first_combination, 1) << "instance is below minimum of 20");

        EXPECT_EQ(error_list[3].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[3].message, combinationError(first_combination, 2) << "unexpected instance type");
    }

    {
        MyErrorHandler error_handler;
        validate(schema, nlohmann::json{ { "first", "short" } }, &error_handler);

        const MyErrorHandler::ErrorEntryList& error_list = error_handler.getErrors();
        EXPECT_EQ(error_list.size(), 4);

        EXPECT_EQ(error_list[0].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[0].message, rootError(first_combination, 3));

        EXPECT_EQ(error_list[1].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[1].message, combinationError(first_combination, 0) << "unexpected instance type");

        EXPECT_EQ(error_list[2].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[2].message, combinationError(first_combination, 1) << "unexpected instance type");

        EXPECT_EQ(error_list[3].ptr, nlohmann::json::json_pointer{ "/first" });
        EXPECT_MATCH(error_list[3].message, combinationError(first_combination, 2) << "instance is too short as per minLength:10");
    }
}

} // namespace <anonymous>

//==============================================================================
// MAIN - calling the test cases
//==============================================================================
auto main() -> int
{
    simpleTest("anyOf", "anyOf");
    simpleTest("anyOf", "oneOf");
    simpleTest("oneOf", "anyOf");
    simpleTest("oneOf", "oneOf");

    verboseTest("anyOf", "anyOf");
    verboseTest("anyOf", "oneOf");
    verboseTest("oneOf", "anyOf");
    verboseTest("oneOf", "oneOf");

    return g_error_count;
}

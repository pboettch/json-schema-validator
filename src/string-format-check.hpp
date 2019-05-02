#pragma once

#include <string>

/**
 * Checks validity of JSON schema built-in string format specifiers like 'date-time', 'ipv4', ...
 */
void string_format_check(const std::string &format, const std::string &value);

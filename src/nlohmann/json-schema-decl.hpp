#ifndef NLOHMANN_JSON_SCHEMA_DECL_HPP__
#define NLOHMANN_JSON_SCHEMA_DECL_HPP__

#ifdef _WIN32
#	if defined(JSON_SCHEMA_VALIDATOR_EXPORTS)
#		define JSON_SCHEMA_VALIDATOR_API __declspec(dllexport)
#	elif defined(JSON_SCHEMA_VALIDATOR_IMPORTS)
#		define JSON_SCHEMA_VALIDATOR_API __declspec(dllimport)
#	else
#		define JSON_SCHEMA_VALIDATOR_API
#	endif
#else
#	define JSON_SCHEMA_VALIDATOR_API
#endif

#endif

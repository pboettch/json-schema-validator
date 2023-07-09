# Roadmap

## Design goals

The main goal of this validator is to produce *human-comprehensible* error
messages if a JSON-document/instance does not comply to its schema.

By default this is done with exceptions thrown at the users with a helpful
message telling what's wrong with the document while validating.

Starting with **2.0.0** the user can pass a `json_schema::basic_error_handler`-derived
object along with the instance to validate to receive a callback each time
a validation error occurs and decide what to do (throwing, counting, collecting).

Another goal was to use Niels Lohmann's JSON-library. This is why the validator
lives in his namespace.

## Thread-safety

Instance validation is thread-safe and the same validator-object can be used by
different threads:

The validate method is `const` which indicates the object is not modified when
being called:

```C++
	json json_validator::validate(const json &) const;
```

Validator-object creation however is not thread-safe. A validator has to be
created in one (main?) thread once.

## Weaknesses

Numerical validation uses nlohmann-json's integer, unsigned and floating point
types, depending on if the schema type is "integer" or "number". Bignum
(i.e. arbitrary precision and range) is not supported at this time.

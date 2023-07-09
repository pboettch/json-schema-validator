# Changelog

## New in version 2

Although significant changes have been done for the 2nd version
(a complete rewrite) the API is compatible with the 1.0.0 release. Except for
the namespace which is now `nlohmann::json_schema`.

Version **2** supports JSON schema draft 7, whereas 1 was supporting draft 4
only. Please update your schemas.

The primary change in 2 is the way a schema is used. While in version 1 the schema was
kept as a JSON-document and used again and again during validation, in version 2 the schema
is parsed into compiled C++ objects which are then used during validation. There are surely
still optimizations to be done, but validation speed has improved by factor 100
or more.

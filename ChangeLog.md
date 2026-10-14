## Release 2.5.0

- Structured validation errors: `error_handler::error(const validation_error &, const json &)`
  reports the failed JSON Schema `keyword` and a machine-readable `details` object alongside the
  existing location, instance and message. The previous three-argument callback remains
  source-compatible and is called by default. `required` and array-valued `dependencies`
  errors identify the missing property without repeating the complete keyword array in every
  error. Fixes #352, related to #321 and #322.
- Added `json_validator::is_valid()` for validity-only checks: one evaluation, no diagnostics,
  no default values.
- ABI change: `error_handler` gains a virtual function. Binaries linked against 2.4.x must be
  rebuilt; the library `SOVERSION` is unchanged.
- Logical combinations (`allOf`, `anyOf`, `oneOf`) evaluate their branches for validity first
  and report the summary error from that result; when the handler does not throw, the failed
  branches are then evaluated again to deliver their individual errors. Errors are no longer
  buffered; custom `format` or content checkers may therefore be called more than once for the
  same value when validation fails. The `allOf` summary is now reported at the combination's
  own location with a fixed message, like `anyOf` and `oneOf`, and every failed `allOf` branch
  is reported, each with its own summary followed by the branch's errors (previously only the
  first failed branch, with its first error embedded in the summary message).
- `anyOf` evaluates all of its branches, so the default values of every successful branch are
  collected (previously only those of the first). JSON Schema does not allow short-circuit
  evaluation while annotations are collected. `is_valid()` still stops at the first successful
  branch.
- `additionalProperties` failures are reported individually at the offending property instead
  of as a single summary at the containing object. `additionalProperties`, `additionalItems`
  and `propertyNames` set to `false` report a direct message.
- Tuple `items` and `additionalItems` failures report the correct array index (previously
  always `/0`).
- Each violated numeric constraint produces a separate error (previously concatenated).

## Release 2.4.1

- Fix the email format check (`is_address`) on platforms where `char` is unsigned (arm64, ppc64el, s390x, ...) by @jamesjer in <https://github.com/pboettch/json-schema-validator/pull/376>
- Use valid pointer ranges for email checks by @Piecha93 in <https://github.com/pboettch/json-schema-validator/pull/382>
- Fix exception slicing in regex format check by @94xhn in <https://github.com/pboettch/json-schema-validator/pull/381>
- Fix compile error: implicit conversion from 'long' to 'double' may lose precision by @VolkerChristian in <https://github.com/pboettch/json-schema-validator/pull/357>
- Fix null values replacing the whole document in the default-value patch by @orgads and @pboettch in <https://github.com/pboettch/json-schema-validator/pull/370>
- Fix crash on '/' or '~' in keys below unknown keywords by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/388>
- Fix recursive `$ref` to a schema below an unknown keyword by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/389>
- Remove dot-segments from URI paths, the same file referenced with different paths is loaded once by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/390>
- Build without nlohmann/json's implicit conversions by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/391>
- Fix the `JSON_VALIDATOR_SHARED_LIBS` option, Windows DLL builds and warning options with clang-cl, add Windows CI by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/392>
- json-schema-validate: load `file://` URIs by their absolute path by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/393>
- Use read-only ifstream to load the schema files in tests by @DonKult in <https://github.com/pboettch/json-schema-validator/pull/379>
- CI: strict warnings build on latest GCC/Clang, fix test warnings by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/387>
- Fix release workflow tag trigger by @LecrisUT in <https://github.com/pboettch/json-schema-validator/pull/369>
- README: fix the CMake target, document installing via FetchContent by @sjoubert, @avaliente-evs and @pboettch in <https://github.com/pboettch/json-schema-validator/pull/386> and <https://github.com/pboettch/json-schema-validator/pull/332>
- README: update nlohmann-json integration, document default values by @pboettch in <https://github.com/pboettch/json-schema-validator/pull/394>

## Release 2.4.0

- Added CI job to publish GitHub release by @JohanMabille in <https://github.com/pboettch/json-schema-validator/pull/367>
- Maintenance to Fedora CI infrastructure by @LecrisUT and @JohanMabille in <https://github.com/pboettch/json-schema-validator/pull/363>
- Reference validation using contains() result rather than exception handling by @BalrogOfHell in <https://github.com/pboettch/json-schema-validator/pull/334>
- add support for $defs instead of definitions by rpatters1 in <https://github.com/pboettch/json-schema-validator/pull/338>
- Apply clang-format / fix "test / Check pre-commit" failures by @serge-s in <https://github.com/pboettch/json-schema-validator/pull/328>
- Adding verbose error messages for logical combinations by Csaba Imre Zempleni in <https://github.com/pboettch/json-schema-validator/pull/310>
- fix: issue-311 by andrejlevkovitch
- Fix cmake install target on windows by @barts-of in <https://github.com/pboettch/json-schema-validator/pull/315>
- error-messages: Numeric limit errors should show maximum precision by @pboettch
- Add Fedora packaging by @LecrisUT in <https://github.com/pboettch/json-schema-validator/pull/264>
- Improve and fix bugs in Conanfile by Jacob Crabill

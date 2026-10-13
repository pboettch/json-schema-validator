# JSON-Schema-Test-Suite

`tests/draft7/` and `remotes/` are plain copies of the official
[JSON-Schema-Test-Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite) (`LICENSE`), commit
`8568bcc477cc3aaeed5bc397c9c8648e631317fc` (2026-10-11). They are excluded from pre-commit, do not edit them.

Each JSON file is one ctest, run by `json-schema-test`. A newer checkout of the suite can be used with
`-DJSON_SCHEMA_TEST_SUITE_PATH=<path>`.

## Known failures

- Optional files which fail completely are marked `WILL_FAIL` in `CMakeLists.txt`.
- Single test cases which fail are listed in `known-failures.json`, by file, group and test description (all tests of
  the group if `tests` is missing), with the reason. The test fails when a listed test case passes or a listed group
  or test case does not exist (anymore), so the list stays up to date.

## Updating

```bash
git clone https://github.com/json-schema-org/JSON-Schema-Test-Suite.git /tmp/suite
rm -r tests/draft7 remotes
cp -r /tmp/suite/tests/draft7 tests/
cp -r /tmp/suite/remotes .
```

Then update the commit above, run the tests and update `known-failures.json`.

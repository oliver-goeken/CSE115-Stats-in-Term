Testing framework is Unity, which is under the MIT License.

GitHub:
- https://github.com/ThrowTheSwitch/Unity

Local license:
- LICENSE.txt

Build all test binaries from the repo root:

	make test

Current test binaries:

- tests/test_cli
- tests/test_display
- tests/test_parse_db_reports
- tests/test_search
- tests/test_spotify_api

Run a single test binary directly, for example:

	./tests/test_search

Clean generated test binaries and macOS dSYM folders:

	make clean

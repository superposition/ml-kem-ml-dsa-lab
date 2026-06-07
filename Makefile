.PHONY: test cpp-test validate-json validate-docs rust-crate-gate clean

test: cpp-test validate-json validate-docs rust-crate-gate

cpp-test:
	cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
	cmake --build build/cpp
	ctest --test-dir build/cpp --output-on-failure

validate-json:
	python3 -m json.tool schemas/agentic-learning.schema.json >/dev/null
	python3 -m json.tool learning/ml-kem-fips203.track.json >/dev/null
	python3 -m json.tool learning/ml-dsa-fips204.track.json >/dev/null
	python3 -m json.tool fixtures/parameter-sets.json >/dev/null
	python3 -m json.tool fixtures/polynomial-examples.json >/dev/null

validate-docs:
	bash scripts/check-docs-glossary.sh

rust-crate-gate:
	bash scripts/check-rust-crate-gate.sh

clean:
	rm -rf build target

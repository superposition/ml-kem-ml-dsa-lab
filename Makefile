.PHONY: test cpp-test rust-test validate-json validate-docs validate-vectors validate-side-channel rust-crate-gate clean

test: cpp-test rust-crate-gate rust-test validate-json validate-docs validate-vectors validate-side-channel

cpp-test:
	cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
	cmake --build build/cpp
	ctest --test-dir build/cpp --output-on-failure

rust-test:
	cargo test --manifest-path rust/pqcore/Cargo.toml

validate-json:
	python3 -m json.tool schemas/agentic-learning.schema.json >/dev/null
	python3 -m json.tool learning/ml-kem-fips203.track.json >/dev/null
	python3 -m json.tool learning/ml-dsa-fips204.track.json >/dev/null
	python3 -m json.tool fixtures/parameter-sets.json >/dev/null
	python3 -m json.tool fixtures/polynomial-examples.json >/dev/null
	python3 -m json.tool fixtures/encoding-examples.json >/dev/null
	python3 -m json.tool fixtures/ntt-examples.json >/dev/null
	python3 -m json.tool fixtures/sampling-examples.json >/dev/null
	python3 -m json.tool fixtures/ml-kem-pke-examples.json >/dev/null
	python3 -m json.tool fixtures/ml-kem-kem-examples.json >/dev/null
	python3 -m json.tool fixtures/ml-dsa-helper-examples.json >/dev/null
	python3 -m json.tool fixtures/ml-dsa-signature-examples.json >/dev/null
	python3 -m json.tool test-vectors/manifest.json >/dev/null
	python3 -m json.tool test-vectors/placeholders/public-api-failclosed.json >/dev/null
	python3 -m json.tool audits/side-channel/manifest.json >/dev/null

validate-docs:
	bash scripts/check-docs-glossary.sh

validate-vectors:
	python3 scripts/check-vector-manifest.py --self-test

validate-side-channel:
	python3 scripts/check-side-channel-manifest.py --self-test

rust-crate-gate:
	bash scripts/check-rust-crate-gate.sh

clean:
	rm -rf build target

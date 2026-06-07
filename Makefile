.PHONY: test cpp-test validate-json clean

test: cpp-test validate-json

cpp-test:
	cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
	cmake --build build/cpp
	ctest --test-dir build/cpp --output-on-failure

validate-json:
	python3 -m json.tool schemas/agentic-learning.schema.json >/dev/null
	python3 -m json.tool learning/ml-kem-fips203.track.json >/dev/null
	python3 -m json.tool learning/ml-dsa-fips204.track.json >/dev/null

clean:
	rm -rf build target

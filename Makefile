.PHONY: test rust-test cpp-test clean

test: rust-test cpp-test

rust-test:
	cargo test --workspace

cpp-test:
	cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
	cmake --build build/cpp
	ctest --test-dir build/cpp --output-on-failure

clean:
	rm -rf build target


# Changelog

## Unreleased

- Initial C++ library skeleton and Rust derivation track.
- Added ML-KEM and ML-DSA parameter metadata.
- Added field and polynomial primitives with tests.
- Added fail-closed C++ public API surfaces for KEM and signature operations.
- Replaced the premature Rust crate with a derivation-first Rust track.
- Added a CI-enforced Rust crate gate and derivation issue template.
- Added glossary, FIPS symbol maps, and docs validation for roadmap modules.
- Added source-linked parameter derivation, shared parameter fixture, and complete C++ parameter tests.
- Added source-linked field-element derivation and complete C++ field tests for both moduli.
- Added source-linked polynomial-ring derivation, shared polynomial fixture format, and complete C++ polynomial tests.
- Added source-linked ML-KEM encoding/compression derivation, shared encoding fixtures, and C++ encoding tests.
- Added source-linked ML-KEM NTT derivation, shared NTT fixtures, and C++ NTT oracle tests.
- Added source-linked sampling derivation, isolated deterministic test hooks, and C++ sampling tests.
- Added source-linked internal ML-KEM-512 PKE derivation and C++ test-hook implementation.
- Added source-linked internal ML-KEM-512 KEM derivation, implicit-rejection tests, and public fail-closed contract.
- Added source-linked ML-DSA rounding, hint, and challenge helper derivation with C++ tests.
- Added source-linked ML-DSA signature-flow derivation, deterministic internal harness, and public fail-closed tests.
- Added agentic learning schema and production readiness gates.

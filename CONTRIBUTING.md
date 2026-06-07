# Contributing

This project is test-driven. Every implementation change should land with tests that identify the relevant standard section or readiness gate.

## Rules

- Do not add production-use claims without completing the readiness gate they depend on.
- Prefer small, spec-mapped commits.
- Keep Rust and C++ behavior aligned with differential tests when possible.
- Mark unsafe or secret-dependent code paths clearly for review.
- Use official ACVP/KAT vectors for algorithm claims.

## Local Checks

```bash
make test
```


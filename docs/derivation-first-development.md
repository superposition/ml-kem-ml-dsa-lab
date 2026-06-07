# Derivation-First Development

This project should teach the implementation into existence.

The goal is not to start with a polished Rust or C++ API and fill in cryptography later. The goal is to derive each module from the standard, state the invariants in plain language, write tests for those invariants, and only then write code.

## Working Loop

1. Read the relevant FIPS section or table.
2. Write a short derivation note.
3. Identify the mathematical object being represented.
4. Write the smallest red test that proves the object is understood.
5. Implement the smallest green behavior.
6. Add edge cases from the standard.
7. Cross-check Rust and C++ once both derivations exist.
8. Promote the behavior into public API only after the internal representation has settled.

## Ticket Shape

Every implementation ticket should include:

- project-manager scope,
- teacher notes,
- first-principles derivation,
- TDD sequence,
- acceptance criteria,
- readiness caveats.

## Why This Matters

For cryptography, a compiling API can be actively misleading. A derivation-first workflow keeps the project honest by tying implementation claims to:

- standard text,
- algebraic invariants,
- official vectors,
- side-channel review,
- reproducible tests.


# FIPS 203 Symbols For ML-KEM

Source of truth: FIPS 203, Module-Lattice-Based Key-Encapsulation Mechanism Standard.

The NIST CSRC FIPS 203 page reports a final publication date of August 13, 2024 and includes a planning note about future errata. Re-check the official page and errata before implementing algorithm details.

## Parameter Sets

| Name | Meaning | Implementation representation | Test invariant |
| --- | --- | --- | --- |
| `ML-KEM-512` | Lowest ML-KEM parameter set in the standard set | `MlKemParameterSet::MlKem512` in C++ | Name, dimensions, and byte lengths match FIPS 203 |
| `ML-KEM-768` | Middle ML-KEM parameter set | `MlKemParameterSet::MlKem768` in C++ | Name, dimensions, and byte lengths match FIPS 203 |
| `ML-KEM-1024` | Highest ML-KEM parameter set | `MlKemParameterSet::MlKem1024` in C++ | Name, dimensions, and byte lengths match FIPS 203 |

## Core Symbols

| Symbol | Plain-language meaning | Representation | Test invariant |
| --- | --- | --- | --- |
| `q` | Coefficient modulus, `3329` | `ModQ<3329>` for field work | All coefficients normalize into `[0, 3329)` |
| `R_q` | Polynomial ring `Z_q[x] / (x^256 + 1)` | Fixed-size polynomial type | Multiplication is negacyclic |
| `k` | Module dimension for vectors and matrices | ML-KEM parameter table | Vector and matrix shapes match the selected set |
| `eta1` | Secret/noise sampling parameter | ML-KEM parameter table and sampler | Sampler output satisfies the bound |
| `eta2` | Error/noise sampling parameter | ML-KEM parameter table and sampler | Sampler output satisfies the bound |
| `du` | Ciphertext compression width for one component | ML-KEM parameter table and compression code | Ciphertext byte length matches parameter set |
| `dv` | Ciphertext compression width for another component | ML-KEM parameter table and compression code | Ciphertext byte length matches parameter set |

## Repository Module Map

| Module | FIPS 203 source pointer | FIPS 203 role | Teaching focus | Follow-up issue |
| --- | --- | --- | --- | --- |
| `params` | Tables 2 and 3 | Parameter sets and byte lengths | Tables are test fixtures from the standard | #3 |
| `field` | Sections 2.4 and 4.2.1 | Arithmetic modulo `q` | Canonical and centered representatives | #4 |
| `polynomial` | Sections 2.4.4 and 2.4.5 | Arithmetic in `R_q` | Negacyclic reduction | #5 |
| `encoding` | Section 4.2.1, Algorithms 3-6 | Byte packing and unpacking | Exact byte lengths and malformed input | #6 |
| `sampling` | Section 4.2.2, Algorithms 7-8, Appendix B Table 4 | Matrix/noise generation | Deterministic expansion versus entropy | #8 |
| `ntt` | Section 4.3, Algorithms 9-12, Appendix A | Fast polynomial multiplication representation | NTT as an oracle-checked representation | #7 |
| `pke` | Section 5, Algorithms 13-15 | Internal public-key encryption layer | KEM is built in layers | #9 |
| `kem` | Sections 6-7, Algorithms 16-21 | Public keygen, encapsulation, decapsulation | Shared secret plus failure behavior | #10 |

## Compatibility Note

Kyber is the legacy/common algorithm-family name. Final FIPS 203 ML-KEM is the repository target. Do not assume test vectors, byte layouts, or APIs from older Kyber drafts are compatible unless the exact revision is named and tested.

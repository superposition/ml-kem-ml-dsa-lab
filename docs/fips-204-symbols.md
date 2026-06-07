# FIPS 204 Symbols For ML-DSA

Source of truth: FIPS 204, Module-Lattice-Based Digital Signature Standard.

The NIST CSRC FIPS 204 page reports a final publication date of August 13, 2024 and includes planning notes about future errata. Re-check the official page and errata before implementing algorithm details.

## Parameter Sets

| Name | Meaning | Implementation representation | Test invariant |
| --- | --- | --- | --- |
| `ML-DSA-44` | Lowest ML-DSA parameter set in the standard set | `MlDsaParameterSet::MlDsa44` in C++ | Name, dimensions, and byte lengths match FIPS 204 |
| `ML-DSA-65` | Middle ML-DSA parameter set | `MlDsaParameterSet::MlDsa65` in C++ | Name, dimensions, and byte lengths match FIPS 204 |
| `ML-DSA-87` | Highest ML-DSA parameter set | `MlDsaParameterSet::MlDsa87` in C++ | Name, dimensions, and byte lengths match FIPS 204 |

## Core Symbols

| Symbol | Plain-language meaning | Representation | Test invariant |
| --- | --- | --- | --- |
| `q` | Coefficient modulus, `8380417` | `ModQ<8380417>` for field work | All coefficients normalize into `[0, 8380417)` |
| `R_q` | Polynomial ring `Z_q[x] / (x^256 + 1)` | Fixed-size polynomial type | Multiplication is negacyclic |
| `k` | One module/matrix dimension | ML-DSA parameter table | Matrix and vector shapes match the selected set |
| `l` | The other module/matrix dimension | ML-DSA parameter table | Secret-vector length matches the selected set |
| `eta` | Small-secret sampling bound | ML-DSA parameter table and sampler | Secret coefficients satisfy the bound |
| `tau` | Challenge polynomial weight | ML-DSA parameter table and challenge sampler | Exactly `tau` nonzero challenge coefficients |
| `gamma1` | Signing mask and bound parameter | ML-DSA parameter table | Signing checks respect the bound |
| `gamma2` | Decomposition and hint parameter | ML-DSA parameter table and rounding code | High/low-bit tests recombine correctly |
| `omega` | Hint-count bound | ML-DSA parameter table and verifier | Signatures with too many hints are rejected |
| `challenge entropy` | Informational challenge entropy value from the parameter table | ML-DSA parameter table | Values are 192, 225, and 257 bits |
| `security category` | Claimed NIST security strength category | ML-DSA parameter table | Categories are 2, 3, and 5 |

## Repository Module Map

| Module | FIPS 204 source pointer | FIPS 204 role | Teaching focus | Follow-up issue |
| --- | --- | --- | --- | --- |
| `params` | Tables 1 and 2 | Parameter sets and byte lengths | Tables are test fixtures from the standard | #3 |
| `field` | Sections 7.1, 7.4, 7.5, and 7.6 | Arithmetic modulo `q` | Canonical and centered representatives | #4 |
| `polynomial` | NTT representation discussion plus Sections 7.5 and 7.6 | Arithmetic in `R_q` | Negacyclic reduction | #5 |
| `encoding` | Sections 7.1 and 7.2, Algorithms 9-28 | Key and signature packing | Exact byte lengths and malformed input | #6 |
| `sampling` | Section 7.3, Algorithms 29-34, Appendix Table 3 | Secret, mask, and challenge sampling | Determinism versus entropy | #8 |
| `ntt` | Sections 7.5 and 7.6, Algorithms 41-48 | Fast polynomial multiplication representation | NTT as an oracle-checked representation | #7 |
| `rounding` | Section 7.4, Algorithms 35-40 | Decomposition and hint routines | How the verifier reconstructs enough information | #11 |
| `dsa` | Sections 5 and 6, Algorithms 1-8 | Public keygen, signing, verification | Signature flow and rejection behavior | #12 |

## Compatibility Note

Dilithium is the legacy/common algorithm-family name. Final FIPS 204 ML-DSA is the repository target. Do not use older Dilithium drafts as conformance evidence unless the exact revision is named and tested.

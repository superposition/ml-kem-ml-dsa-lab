# Constant-Time and Side-Channel Review Gate

This gate tracks secret-bearing behavior for the current C++ implementation and the future Rust
implementation. It does not claim production constant-time behavior.

The machine-readable review state is:

```text
audits/side-channel/manifest.json
```

Validate it with:

```bash
python3 scripts/check-side-channel-manifest.py --self-test
```

The strict production gate is intentionally still closed:

```bash
python3 scripts/check-side-channel-manifest.py --require-complete
```

That command must fail until final implementations, official vectors, optimized compiler-output review,
and external cryptographic review are complete.

## Review Rules

- Public shape checks may branch on public lengths, parameter sets, and malformed public inputs.
- Secret-dependent validity must not produce a distinct error class.
- Secret-bearing byte strings must not expose raw bytes through default debug output.
- Secret-bearing selection must use mask-based selection or another reviewed constant-time pattern.
- Rejection loops must be reviewed in the context of the final algorithm and compiler output.
- Timing smoke tests are advisory only. They are useful signals, not proofs.

## Debug Policy

C++ secret annotations use `pqcore::SecretBytesView` from:

```text
cpp/include/pqcore/secret_data.hpp
```

`SecretBytesView` is a redacting view for labels, sizes, and kind names. It is not an ownership type
and does not zero memory. Its purpose is to prevent future review notes and diagnostics from printing
secret bytes by accident.

The test `test_secret_data` checks that formatted secret byte views contain metadata and the word
`<redacted>`, but not the underlying byte values.

## Module Notes

### Field, Polynomial, and NTT

Public inputs:

- modulus metadata,
- public polynomial fixtures.

Secret inputs:

- secret coefficients when the shared types are used by K-PKE, KEM, or ML-DSA.

Side-channel note: these shared arithmetic types are correctness scaffolding. Value-dependent
normalization branches and compiler output are not production-reviewed. Once these paths carry secret
coefficients in a final implementation, branch behavior, memory access, table access, and optimized
codegen must be reviewed.

### Encoding and Compression

Public inputs:

- public keys,
- ciphertexts,
- signatures,
- bit widths.

Secret inputs:

- secret-key encodings once ML-DSA key packing lands.

Side-channel note: current ML-KEM encoding covers public byte contracts. Secret-key encoders and
decoders must get separate side-channel review when they are added. Secret-key parsing must not leak
secret validity through a distinct error class.

### Sampling

Public inputs:

- public sampler parameters,
- challenge bytes.

Secret inputs:

- seeds,
- PRF inputs,
- secret-vector expansion input,
- signing mask input.

Side-channel note: rejection samplers branch on candidate values. Current tests prove bounds,
byte-stream behavior, and SHAKE-backed PRF/XOF wrapper shape. Secret-bearing production sampler loops
remain blockers.
If a production sampler uses bounded rejection, failure handling must destroy intermediates.

### ML-KEM K-PKE

Public inputs:

- encryption key,
- ciphertext,
- message length.

Secret inputs:

- key-generation seed,
- `sigma`,
- decryption key,
- encryption coins.

Secret-derived intermediates:

- `s`,
- `e`,
- `y`,
- `e1`,
- `e2`,
- secret NTT vector,
- decrypted message.

Side-channel note: K-PKE now has SHAKE-backed seed, matrix, and noise expansion helpers, but the
end-to-end K-PKE path remains a deterministic test hook. It preserves the algorithm shape, but it is
not production cryptography and is not production-reviewed for side channels.

### ML-KEM KEM

Public inputs:

- encapsulation key,
- ciphertext,
- parameter set.

Secret inputs:

- decapsulation key,
- PKE decryption key,
- implicit-rejection seed,
- candidate shared secret,
- fallback shared secret.

Secret-derived intermediates:

- recovered message,
- reencryption coins,
- reencrypted ciphertext,
- valid-ciphertext mask.

Decapsulation failure handling review: the internal ML-KEM-512 test hook checks ciphertext equality by
accumulating a full-length diff, derives a mask, and selects between the candidate and fallback shared
secret with byte masks. A malformed but well-sized ciphertext returns a fallback shared secret rather
than a distinct secret-dependent error. This is reviewed for the internal test hook only.

Production blockers:

- official vector execution,
- optimized compiler-output review,
- external cryptographic review.

### ML-DSA Helpers

Public inputs:

- parameter set,
- challenge seed bytes.

Secret inputs:

- low bits and hint inputs in full signing paths.

Secret-derived intermediates:

- decomposed coefficients,
- hints,
- challenge-dependent products.

Side-channel note: helper tests prove algebraic behavior. They do not prove that secret-bearing
signing use is constant-time.

### ML-DSA Signature Flow

Public inputs:

- public key,
- message,
- context,
- signature,
- parameter set.

Secret inputs:

- secret key,
- signing seed `K`,
- signing randomness,
- signing mask seed.

Secret-derived intermediates:

- message representative,
- `rho_second`,
- witness,
- response vector,
- hint vector.

Signing rejection behavior review: SHAKE256-backed helpers now cover `tr`, `mu`, `rho_second`, and
challenge bytes. The current signing loop is still a deterministic flow harness. It intentionally
exercises a rejection before a deterministic acceptance so tests can inspect the API boundary. This is
not production signing behavior, and public signing remains fail-closed.

Production blockers:

- final ML-DSA key packing,
- final NTT and expansion plumbing,
- official vector execution,
- side-channel review of rejection loops and hint handling,
- optimized compiler-output review.

### Public API Stubs

Public inputs:

- parameter set,
- public key,
- ciphertext,
- message,
- context,
- signature.

Secret inputs:

- secret key byte strings provided to signing or decapsulation.

Side-channel note: public ML-KEM and ML-DSA APIs remain fail-closed. They perform public length checks
and then throw `NotImplemented`; they do not execute a production secret-bearing algorithm yet.

## Finding Tracking

The current production blockers are tracked in the manifest:

- `SC-KEM-DEC-001`,
- `SC-DSA-SIGN-001`,
- `SC-SAMPLING-001`,
- `SC-COMPILER-001`.

These findings must remain publicly tracked and must be resolved before any production-readiness claim.

# Encoding And Compression Derivation

This document derives the first byte-level encoding and compression primitives used by ML-KEM and records the ML-DSA encoding surface for future implementation.

## Source Pointers

| Scheme | FIPS source | Encoding source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Section 4.2.1, including Algorithms 5 and 6 | C++ `ByteEncode`, `ByteDecode`, `Compress`, and `Decompress` primitives |
| ML-DSA | FIPS 204 | Section 7.2, Algorithms 22-28 | Documentation and future Rust tests only |

## ByteEncode And ByteDecode

FIPS 203 encodes exactly 256 coefficients at a time. For a width `d`, the encoded byte length is:

```text
256 * d bits = 32 * d bytes
```

Bits are written least-significant bit first for each coefficient. `ByteEncode_d` packs the coefficient bits into a flat bit array, then converts that bit array into bytes.

For `1 <= d <= 11`, the coefficient domain is `[0, 2^d)`, and `ByteDecode_d(ByteEncode_d(x)) = x`.

For `d = 12`, the coefficient domain for encoding is `[0, q)`, where `q = 3329`. Decoding reads a 12-bit integer and reduces it modulo `q`. This means malformed or non-canonical 12-bit byte strings can decode to valid field values, but byte strings produced by `ByteEncode_12` round trip exactly.

Test invariants:

- encoded polynomial length is `32 * d`,
- valid `d < 12` values round trip exactly,
- valid `d = 12` ML-KEM coefficients round trip exactly,
- malformed byte lengths are rejected,
- out-of-domain encoding inputs are rejected.

## ML-KEM Key And Ciphertext Lengths

ML-KEM public keys contain `k` encoded 12-bit polynomials plus a 32-byte seed:

```text
32 + k * ByteEncode_12 = 32 + k * 384
```

ML-KEM ciphertexts contain `k` polynomials encoded at width `du` plus one polynomial encoded at width `dv`:

```text
k * ByteEncode_du + ByteEncode_dv
```

These formulas match the parameter table byte lengths.

## Compression And Decompression

ML-KEM compression maps a field element modulo `q = 3329` to `d` bits:

```text
Compress_d(x) = round((2^d / q) * x) mod 2^d
```

Decompression maps the compressed value back into the field approximately:

```text
Decompress_d(y) = round((q / 2^d) * y)
```

Compression is lossy when `d < 12`. Tests must not assert exact round trips except at values where the formula actually produces them.

Rounding is performed over rational numbers with integer arithmetic. Floating-point arithmetic is not used.

Test invariants:

- `Compress_d(0) = 0`,
- `Compress_d(Decompress_d(y)) = y`,
- endpoints wrap as specified by the modulo `2^d` output domain,
- decompressed values are canonical field elements,
- the centered error after `Decompress_d(Compress_d(x))` stays within the expected quantization bound.

## Serialization Bugs Versus Algebra Bugs

Encoding bugs are byte-contract bugs. They include wrong lengths, wrong bit order, malformed input handling, or accidental acceptance of out-of-domain values.

Algebra bugs are mathematical-object bugs. They include wrong modular arithmetic, wrong polynomial reduction, or wrong NTT behavior.

The distinction matters because a valid algebraic operation can still produce a non-standard byte string if the packing rule is wrong.

## ML-DSA Encoding Notes

FIPS 204 Section 7.2 defines public-key, secret-key, signature, hint, and `w1` encodings. Several ML-DSA decoders are run on untrusted public data and must handle malformed inputs explicitly.

This ticket does not implement those ML-DSA packers yet. Future ML-DSA encoding work should derive `SimpleBitPack`, `BitPack`, `HintBitPack`, and their unpacking routines with separate malformed-input tests.

## Vector-Loader Compatibility

The fixture at `fixtures/encoding-examples.json` is repository-owned derivation data, not an official vector file. ACVP/KAT vector ingestion must keep official source metadata and should distinguish:

- raw byte fixtures,
- parsed coefficient arrays,
- expected rejection cases,
- approximate compression expectations.

## Constant-Time And Parser Caveat

Encoding is parser attack surface. Decoder length checks and malformed-input behavior must be fuzzed before production readiness.

Secret-bearing byte arrays should not be printed by default. Future formatted debug output must redact or omit secret bytes.

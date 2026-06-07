# Rust Derivation: Encoding And Compression

The initial Rust crate exists, but this module remains future derivation work.

The future Rust implementation should derive encoding from `docs/encoding-and-compression.md` and the shared fixture at `fixtures/encoding-examples.json`.

## Future Type Shape

ML-KEM byte encoding should be parameterized by bit width:

```text
byte_encode<const D: usize>(coefficients: [i32; 256]) -> [u8; 32 * D]
byte_decode<const D: usize>(bytes: [u8; 32 * D]) -> [i32; 256]
```

The Rust implementation may use vectors until const-generic arithmetic is ergonomic, but the derivation target is fixed-size byte contracts.

## Future Red Tests

1. `ByteEncode_4` produces 128 bytes.
2. `ByteEncode_10` produces 320 bytes.
3. `ByteEncode_11` produces 352 bytes.
4. `ByteEncode_12` produces 384 bytes.
5. `ByteDecode_4(ByteEncode_4(x)) = x` for values in `[0, 16)`.
6. `ByteDecode_12(ByteEncode_12(x)) = x` for values in `[0, 3329)`.
7. `ByteDecode_12` reduces non-canonical 12-bit segments modulo `3329`.
8. Decoding reports malformed lengths.
9. Encoding rejects out-of-domain values.
10. ML-KEM public-key and ciphertext byte-length formulas match the parameter table.
11. `Compress_d` maps boundary values correctly.
12. `Decompress_d` maps compressed endpoints into canonical field elements.
13. `Decompress_d(Compress_d(x))` stays within the expected centered error bound.
14. Secret-bearing byte containers do not expose raw bytes in default debug output.

## ML-DSA Follow-Up

Future ML-DSA encoding work should derive FIPS 204 `SimpleBitPack`, `BitPack`, `HintBitPack`, public-key encoding, secret-key encoding, signature encoding, and `w1Encode` separately.

Public-key and signature decoders are untrusted-input parsers. Secret-key decoding is only safe for trusted inputs where the standard says malformed values can escape the intended range.

## Vector-Loader Note

The future Rust vector loader should keep official ACVP/KAT data separate from repository-owned derivation examples. Compression fixtures must encode approximate expectations instead of false exact round trips.

## Readiness Caveat

Correct byte packing is necessary for standards compatibility, but decoders still need fuzzing and side-channel review before production use.

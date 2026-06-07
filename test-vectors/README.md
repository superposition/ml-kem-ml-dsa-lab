# Test Vectors

Official ACVP/KAT vector ingestion is tracked here.

The repository should store scripts or small manifest files rather than large generated vector dumps unless there is a clear release reason to vendor them. The Phase 6 vector gate vendors the NIST ACVP-Server generated prompt and expected-results files because the strict gate now depends on local hash-checked evidence.

Current files:

- `manifest.json` records source provenance, retrieval dates, hashes, vector-set status, and pending reasons.
- `acvp/` contains vendored NIST ACVP-Server generated JSON prompt and expected-results pairs for ML-KEM and ML-DSA.
- `placeholders/public-api-failclosed.json` is a repository-owned harness fixture. It is not an official vector file.

Run:

```bash
python3 scripts/check-vector-manifest.py --self-test --require-official
```

The strict gate checks local file hashes, ACVP top-level metadata, parameter-set coverage, group counts, and case counts. Passing it proves vector provenance and parser coverage. It does not prove that the public ML-KEM or ML-DSA algorithms pass the vectors yet.

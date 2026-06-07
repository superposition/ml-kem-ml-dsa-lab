# Test Vectors

Official ACVP/KAT vector ingestion is tracked here.

The repository should store scripts or small manifest files rather than large generated vector dumps unless there is a clear release reason to vendor them.

Current files:

- `manifest.json` records source provenance, retrieval dates, hashes, vector-set status, and pending reasons.
- `placeholders/public-api-failclosed.json` is a repository-owned harness fixture. It is not an official vector file.

Run:

```bash
python3 scripts/check-vector-manifest.py --self-test
```

Use `--require-official` when the release gate should fail unless at least one official vector set is vendored.

# Security Policy

This repository is not yet approved for production cryptography.

Report suspected vulnerabilities through GitHub private vulnerability reporting once enabled, or by opening a minimal public issue that does not include exploit details.

Before a production-ready release, this project requires:

- Complete FIPS 203 and FIPS 204 algorithm coverage.
- Official vector coverage.
- Constant-time review for all operations that touch secret data.
- Fuzzing and sanitizer coverage.
- Dependency and build provenance review.
- External cryptographic review.


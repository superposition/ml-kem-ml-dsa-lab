#!/usr/bin/env bash
set -euo pipefail

glossary="docs/glossary.md"
kem_symbols="docs/fips-203-symbols.md"
dsa_symbols="docs/fips-204-symbols.md"

if [[ ! -f "$glossary" ]]; then
  echo "Missing $glossary" >&2
  exit 1
fi

missing=0

while IFS= read -r module; do
  if ! grep -Fq "### \`$module\`" "$glossary"; then
    echo "Missing glossary entry for roadmap module: $module" >&2
    missing=1
  fi
done < <(
  grep -hE '^- `[^`]+`:' docs/fips-203-ml-kem-roadmap.md docs/fips-204-ml-dsa-roadmap.md |
    sed -E 's/^- `([^`]+)`.*/\1/' |
    sort -u
)

while IFS= read -r module; do
  if ! grep -Fq "| \`$module\` |" "$kem_symbols"; then
    echo "Missing FIPS 203 symbol-map row for module: $module" >&2
    missing=1
  fi
done < <(
  grep -hE '^- `[^`]+`:' docs/fips-203-ml-kem-roadmap.md |
    sed -E 's/^- `([^`]+)`.*/\1/' |
    sort -u
)

while IFS= read -r module; do
  if ! grep -Fq "| \`$module\` |" "$dsa_symbols"; then
    echo "Missing FIPS 204 symbol-map row for module: $module" >&2
    missing=1
  fi
done < <(
  grep -hE '^- `[^`]+`:' docs/fips-204-ml-dsa-roadmap.md |
    sed -E 's/^- `([^`]+)`.*/\1/' |
    sort -u
)

required_terms=(
  ML-KEM
  ML-DSA
  Kyber
  Dilithium
  q
  Z_q
  R_q
  coefficient
  polynomial
  vector
  matrix
  k
  l
  eta
  seed
  XOF
  PRF
  sampler
  encoding
  decoding
  compression
  decompression
  "coefficient domain"
  "NTT domain"
  "public input"
  "secret-bearing input"
)

for term in "${required_terms[@]}"; do
  if ! grep -Fq "### \`$term\`" "$glossary"; then
    echo "Missing glossary entry for required term: $term" >&2
    missing=1
  fi
done

if [[ "$missing" -ne 0 ]]; then
  exit 1
fi

echo "Glossary and FIPS symbol maps cover roadmap modules and required notation."

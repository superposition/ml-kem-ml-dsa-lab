# MLS Integration Notes

If MLS means Messaging Layer Security, this repository is not an MLS implementation. It can eventually provide cryptographic building blocks for protocols that negotiate ML-KEM or ML-DSA, but protocol binding work should live behind separate tests.

Future integration work should define:

- Which protocol draft or RFC is being targeted.
- Whether ML-KEM is used directly or through HPKE/hybrid KEM composition.
- How public keys, ciphertexts, and signatures are serialized.
- How downgrade resistance is handled.
- Which interoperability vectors prove the binding.


//! Test-driven ML-KEM and ML-DSA library workbench.
//!
//! This crate intentionally fails closed for full KEM and signature operations
//! until the FIPS algorithms and official vectors are implemented.

pub mod dsa;
pub mod field;
pub mod kem;
pub mod params;
pub mod polynomial;

pub use dsa::{ml_dsa_keygen, ml_dsa_sign, ml_dsa_verify, MlDsaKeyPair, Signature};
pub use kem::{
    ml_kem_decapsulate, ml_kem_encapsulate, ml_kem_keygen, MlKemCiphertext, MlKemKeyPair,
    SharedSecret,
};
pub use params::{MlDsaParameterSet, MlKemParameterSet};

pub const PRODUCTION_READY: bool = false;

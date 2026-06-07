use crate::params::MlKemParameterSet;
use core::fmt;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct MlKemKeyPair {
    pub public_key: Vec<u8>,
    pub secret_key: Vec<u8>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct MlKemCiphertext(pub Vec<u8>);

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SharedSecret(pub [u8; 32]);

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum CryptoError {
    NotImplemented(&'static str),
    InvalidInput(&'static str),
}

impl fmt::Display for CryptoError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::NotImplemented(message) => write!(f, "not implemented: {message}"),
            Self::InvalidInput(message) => write!(f, "invalid input: {message}"),
        }
    }
}

impl std::error::Error for CryptoError {}

pub type Result<T> = core::result::Result<T, CryptoError>;

pub fn ml_kem_keygen(parameter_set: MlKemParameterSet) -> Result<MlKemKeyPair> {
    let _params = parameter_set.params();
    Err(CryptoError::NotImplemented(
        "ML-KEM key generation requires FIPS 203 algorithm and vector coverage",
    ))
}

pub fn ml_kem_encapsulate(
    parameter_set: MlKemParameterSet,
    public_key: &[u8],
) -> Result<(MlKemCiphertext, SharedSecret)> {
    let params = parameter_set.params();
    if public_key.len() != params.public_key_bytes {
        return Err(CryptoError::InvalidInput("public key has the wrong length"));
    }

    Err(CryptoError::NotImplemented(
        "ML-KEM encapsulation requires FIPS 203 algorithm and vector coverage",
    ))
}

pub fn ml_kem_decapsulate(
    parameter_set: MlKemParameterSet,
    secret_key: &[u8],
    ciphertext: &[u8],
) -> Result<SharedSecret> {
    let params = parameter_set.params();
    if secret_key.len() != params.secret_key_bytes {
        return Err(CryptoError::InvalidInput("secret key has the wrong length"));
    }
    if ciphertext.len() != params.ciphertext_bytes {
        return Err(CryptoError::InvalidInput("ciphertext has the wrong length"));
    }

    Err(CryptoError::NotImplemented(
        "ML-KEM decapsulation requires FIPS 203 algorithm and vector coverage",
    ))
}

#[cfg(test)]
mod tests {
    use super::{ml_kem_decapsulate, ml_kem_encapsulate, ml_kem_keygen, CryptoError};
    use crate::params::MlKemParameterSet;

    #[test]
    fn ml_kem_api_fails_closed_until_implemented() {
        assert!(matches!(
            ml_kem_keygen(MlKemParameterSet::MlKem512),
            Err(CryptoError::NotImplemented(_))
        ));

        let params = MlKemParameterSet::MlKem512.params();
        let public_key = vec![0_u8; params.public_key_bytes];
        assert!(matches!(
            ml_kem_encapsulate(MlKemParameterSet::MlKem512, &public_key),
            Err(CryptoError::NotImplemented(_))
        ));

        let secret_key = vec![0_u8; params.secret_key_bytes];
        let ciphertext = vec![0_u8; params.ciphertext_bytes];
        assert!(matches!(
            ml_kem_decapsulate(MlKemParameterSet::MlKem512, &secret_key, &ciphertext),
            Err(CryptoError::NotImplemented(_))
        ));
    }

    #[test]
    fn validates_lengths_before_algorithm_dispatch() {
        assert!(matches!(
            ml_kem_encapsulate(MlKemParameterSet::MlKem768, &[0_u8; 1]),
            Err(CryptoError::InvalidInput(_))
        ));
    }
}

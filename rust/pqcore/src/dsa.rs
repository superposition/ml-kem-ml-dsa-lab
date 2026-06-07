use crate::params::MlDsaParameterSet;
use core::fmt;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct MlDsaKeyPair {
    pub public_key: Vec<u8>,
    pub secret_key: Vec<u8>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Signature(pub Vec<u8>);

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

pub fn ml_dsa_keygen(parameter_set: MlDsaParameterSet) -> Result<MlDsaKeyPair> {
    let _params = parameter_set.params();
    Err(CryptoError::NotImplemented(
        "ML-DSA key generation requires FIPS 204 algorithm and vector coverage",
    ))
}

pub fn ml_dsa_sign(
    parameter_set: MlDsaParameterSet,
    secret_key: &[u8],
    message: &[u8],
    context: &[u8],
) -> Result<Signature> {
    let params = parameter_set.params();
    if secret_key.len() != params.secret_key_bytes {
        return Err(CryptoError::InvalidInput("secret key has the wrong length"));
    }
    if context.len() > 255 {
        return Err(CryptoError::InvalidInput(
            "context is longer than 255 bytes",
        ));
    }
    let _message = message;

    Err(CryptoError::NotImplemented(
        "ML-DSA signing requires FIPS 204 algorithm and vector coverage",
    ))
}

pub fn ml_dsa_verify(
    parameter_set: MlDsaParameterSet,
    public_key: &[u8],
    message: &[u8],
    signature: &[u8],
    context: &[u8],
) -> Result<bool> {
    let params = parameter_set.params();
    if public_key.len() != params.public_key_bytes {
        return Err(CryptoError::InvalidInput("public key has the wrong length"));
    }
    if signature.len() != params.signature_bytes {
        return Err(CryptoError::InvalidInput("signature has the wrong length"));
    }
    if context.len() > 255 {
        return Err(CryptoError::InvalidInput(
            "context is longer than 255 bytes",
        ));
    }
    let _message = message;

    Err(CryptoError::NotImplemented(
        "ML-DSA verification requires FIPS 204 algorithm and vector coverage",
    ))
}

#[cfg(test)]
mod tests {
    use super::{ml_dsa_keygen, ml_dsa_sign, ml_dsa_verify, CryptoError};
    use crate::params::MlDsaParameterSet;

    #[test]
    fn ml_dsa_api_fails_closed_until_implemented() {
        assert!(matches!(
            ml_dsa_keygen(MlDsaParameterSet::MlDsa44),
            Err(CryptoError::NotImplemented(_))
        ));

        let params = MlDsaParameterSet::MlDsa44.params();
        let secret_key = vec![0_u8; params.secret_key_bytes];
        assert!(matches!(
            ml_dsa_sign(
                MlDsaParameterSet::MlDsa44,
                &secret_key,
                b"message",
                b"context"
            ),
            Err(CryptoError::NotImplemented(_))
        ));

        let public_key = vec![0_u8; params.public_key_bytes];
        let signature = vec![0_u8; params.signature_bytes];
        assert!(matches!(
            ml_dsa_verify(
                MlDsaParameterSet::MlDsa44,
                &public_key,
                b"message",
                &signature,
                b"context"
            ),
            Err(CryptoError::NotImplemented(_))
        ));
    }

    #[test]
    fn validates_dsa_context_length() {
        let params = MlDsaParameterSet::MlDsa65.params();
        let secret_key = vec![0_u8; params.secret_key_bytes];
        let context = vec![0_u8; 256];

        assert!(matches!(
            ml_dsa_sign(
                MlDsaParameterSet::MlDsa65,
                &secret_key,
                b"message",
                &context
            ),
            Err(CryptoError::InvalidInput(_))
        ));
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum MlKemParameterSet {
    MlKem512,
    MlKem768,
    MlKem1024,
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct MlKemParams {
    pub name: &'static str,
    pub k: usize,
    pub q: i32,
    pub eta1: usize,
    pub eta2: usize,
    pub du: usize,
    pub dv: usize,
    pub public_key_bytes: usize,
    pub secret_key_bytes: usize,
    pub ciphertext_bytes: usize,
    pub shared_secret_bytes: usize,
}

impl MlKemParameterSet {
    pub const fn params(self) -> MlKemParams {
        match self {
            Self::MlKem512 => MlKemParams {
                name: "ML-KEM-512",
                k: 2,
                q: 3329,
                eta1: 3,
                eta2: 2,
                du: 10,
                dv: 4,
                public_key_bytes: 800,
                secret_key_bytes: 1632,
                ciphertext_bytes: 768,
                shared_secret_bytes: 32,
            },
            Self::MlKem768 => MlKemParams {
                name: "ML-KEM-768",
                k: 3,
                q: 3329,
                eta1: 2,
                eta2: 2,
                du: 10,
                dv: 4,
                public_key_bytes: 1184,
                secret_key_bytes: 2400,
                ciphertext_bytes: 1088,
                shared_secret_bytes: 32,
            },
            Self::MlKem1024 => MlKemParams {
                name: "ML-KEM-1024",
                k: 4,
                q: 3329,
                eta1: 2,
                eta2: 2,
                du: 11,
                dv: 5,
                public_key_bytes: 1568,
                secret_key_bytes: 3168,
                ciphertext_bytes: 1568,
                shared_secret_bytes: 32,
            },
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum MlDsaParameterSet {
    MlDsa44,
    MlDsa65,
    MlDsa87,
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct MlDsaParams {
    pub name: &'static str,
    pub k: usize,
    pub l: usize,
    pub q: i32,
    pub d: usize,
    pub tau: usize,
    pub lambda_bits: usize,
    pub gamma1: i32,
    pub gamma2: i32,
    pub eta: usize,
    pub beta: usize,
    pub omega: usize,
    pub public_key_bytes: usize,
    pub secret_key_bytes: usize,
    pub signature_bytes: usize,
}

impl MlDsaParameterSet {
    pub const fn params(self) -> MlDsaParams {
        match self {
            Self::MlDsa44 => MlDsaParams {
                name: "ML-DSA-44",
                k: 4,
                l: 4,
                q: 8_380_417,
                d: 13,
                tau: 39,
                lambda_bits: 128,
                gamma1: 1 << 17,
                gamma2: (8_380_417 - 1) / 88,
                eta: 2,
                beta: 78,
                omega: 80,
                public_key_bytes: 1312,
                secret_key_bytes: 2560,
                signature_bytes: 2420,
            },
            Self::MlDsa65 => MlDsaParams {
                name: "ML-DSA-65",
                k: 6,
                l: 5,
                q: 8_380_417,
                d: 13,
                tau: 49,
                lambda_bits: 192,
                gamma1: 1 << 19,
                gamma2: (8_380_417 - 1) / 32,
                eta: 4,
                beta: 196,
                omega: 55,
                public_key_bytes: 1952,
                secret_key_bytes: 4032,
                signature_bytes: 3309,
            },
            Self::MlDsa87 => MlDsaParams {
                name: "ML-DSA-87",
                k: 8,
                l: 7,
                q: 8_380_417,
                d: 13,
                tau: 60,
                lambda_bits: 256,
                gamma1: 1 << 19,
                gamma2: (8_380_417 - 1) / 32,
                eta: 2,
                beta: 120,
                omega: 75,
                public_key_bytes: 2592,
                secret_key_bytes: 4896,
                signature_bytes: 4627,
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{MlDsaParameterSet, MlKemParameterSet};

    #[test]
    fn exposes_ml_kem_parameter_sets() {
        let kem768 = MlKemParameterSet::MlKem768.params();

        assert_eq!(kem768.name, "ML-KEM-768");
        assert_eq!(kem768.k, 3);
        assert_eq!(kem768.q, 3329);
        assert_eq!(kem768.public_key_bytes, 1184);
        assert_eq!(kem768.ciphertext_bytes, 1088);
        assert_eq!(kem768.shared_secret_bytes, 32);
    }

    #[test]
    fn exposes_ml_dsa_parameter_sets() {
        let dsa65 = MlDsaParameterSet::MlDsa65.params();

        assert_eq!(dsa65.name, "ML-DSA-65");
        assert_eq!(dsa65.k, 6);
        assert_eq!(dsa65.l, 5);
        assert_eq!(dsa65.q, 8_380_417);
        assert_eq!(dsa65.public_key_bytes, 1952);
        assert_eq!(dsa65.secret_key_bytes, 4032);
        assert_eq!(dsa65.signature_bytes, 3309);
    }
}

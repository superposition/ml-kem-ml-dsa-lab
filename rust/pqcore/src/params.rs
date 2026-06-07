#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum MlKemParameterSet {
    MlKem512,
    MlKem768,
    MlKem1024,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum MlDsaParameterSet {
    MlDsa44,
    MlDsa65,
    MlDsa87,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
struct MlKemParams {
    name: &'static str,
    n: usize,
    q: i32,
    k: usize,
    eta1: usize,
    eta2: usize,
    du: usize,
    dv: usize,
    rbg_strength_bits: usize,
    security_category: usize,
    public_key_bytes: usize,
    secret_key_bytes: usize,
    ciphertext_bytes: usize,
    shared_secret_bytes: usize,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
struct MlDsaParams {
    name: &'static str,
    n: usize,
    q: i32,
    zeta: i32,
    d: usize,
    tau: usize,
    lambda_bits: usize,
    gamma1: i32,
    gamma2: i32,
    k: usize,
    l: usize,
    eta: usize,
    beta: usize,
    omega: usize,
    challenge_entropy_bits: usize,
    security_category: usize,
    public_key_bytes: usize,
    secret_key_bytes: usize,
    signature_bytes: usize,
}

fn ml_kem_params(parameter_set: MlKemParameterSet) -> MlKemParams {
    match parameter_set {
        MlKemParameterSet::MlKem512 => MlKemParams {
            name: "ML-KEM-512",
            n: 256,
            q: 3329,
            k: 2,
            eta1: 3,
            eta2: 2,
            du: 10,
            dv: 4,
            rbg_strength_bits: 128,
            security_category: 1,
            public_key_bytes: 800,
            secret_key_bytes: 1632,
            ciphertext_bytes: 768,
            shared_secret_bytes: 32,
        },
        MlKemParameterSet::MlKem768 => MlKemParams {
            name: "ML-KEM-768",
            n: 256,
            q: 3329,
            k: 3,
            eta1: 2,
            eta2: 2,
            du: 10,
            dv: 4,
            rbg_strength_bits: 192,
            security_category: 3,
            public_key_bytes: 1184,
            secret_key_bytes: 2400,
            ciphertext_bytes: 1088,
            shared_secret_bytes: 32,
        },
        MlKemParameterSet::MlKem1024 => MlKemParams {
            name: "ML-KEM-1024",
            n: 256,
            q: 3329,
            k: 4,
            eta1: 2,
            eta2: 2,
            du: 11,
            dv: 5,
            rbg_strength_bits: 256,
            security_category: 5,
            public_key_bytes: 1568,
            secret_key_bytes: 3168,
            ciphertext_bytes: 1568,
            shared_secret_bytes: 32,
        },
    }
}

fn ml_dsa_params(parameter_set: MlDsaParameterSet) -> MlDsaParams {
    match parameter_set {
        MlDsaParameterSet::MlDsa44 => MlDsaParams {
            name: "ML-DSA-44",
            n: 256,
            q: 8_380_417,
            zeta: 1753,
            d: 13,
            tau: 39,
            lambda_bits: 128,
            gamma1: 131_072,
            gamma2: 95_232,
            k: 4,
            l: 4,
            eta: 2,
            beta: 78,
            omega: 80,
            challenge_entropy_bits: 192,
            security_category: 2,
            public_key_bytes: 1312,
            secret_key_bytes: 2560,
            signature_bytes: 2420,
        },
        MlDsaParameterSet::MlDsa65 => MlDsaParams {
            name: "ML-DSA-65",
            n: 256,
            q: 8_380_417,
            zeta: 1753,
            d: 13,
            tau: 49,
            lambda_bits: 192,
            gamma1: 524_288,
            gamma2: 261_888,
            k: 6,
            l: 5,
            eta: 4,
            beta: 196,
            omega: 55,
            challenge_entropy_bits: 225,
            security_category: 3,
            public_key_bytes: 1952,
            secret_key_bytes: 4032,
            signature_bytes: 3309,
        },
        MlDsaParameterSet::MlDsa87 => MlDsaParams {
            name: "ML-DSA-87",
            n: 256,
            q: 8_380_417,
            zeta: 1753,
            d: 13,
            tau: 60,
            lambda_bits: 256,
            gamma1: 524_288,
            gamma2: 261_888,
            k: 8,
            l: 7,
            eta: 2,
            beta: 120,
            omega: 75,
            challenge_entropy_bits: 257,
            security_category: 5,
            public_key_bytes: 2592,
            secret_key_bytes: 4896,
            signature_bytes: 4627,
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const PARAMETER_FIXTURE: &str = include_str!("../../../fixtures/parameter-sets.json");

    #[test]
    fn ml_kem_parameter_tables_match_fips_names_and_sizes() {
        let kem512 = ml_kem_params(MlKemParameterSet::MlKem512);
        assert_eq!(kem512.name, "ML-KEM-512");
        assert_eq!(kem512.n, 256);
        assert_eq!(kem512.q, 3329);
        assert_eq!(kem512.k, 2);
        assert_eq!(kem512.eta1, 3);
        assert_eq!(kem512.eta2, 2);
        assert_eq!(kem512.du, 10);
        assert_eq!(kem512.dv, 4);
        assert_eq!(kem512.rbg_strength_bits, 128);
        assert_eq!(kem512.security_category, 1);
        assert_eq!(kem512.public_key_bytes, 800);
        assert_eq!(kem512.secret_key_bytes, 1632);
        assert_eq!(kem512.ciphertext_bytes, 768);
        assert_eq!(kem512.shared_secret_bytes, 32);

        let kem768 = ml_kem_params(MlKemParameterSet::MlKem768);
        assert_eq!(kem768.name, "ML-KEM-768");
        assert_eq!(kem768.k, 3);
        assert_eq!(kem768.eta1, 2);
        assert_eq!(kem768.public_key_bytes, 1184);
        assert_eq!(kem768.secret_key_bytes, 2400);
        assert_eq!(kem768.ciphertext_bytes, 1088);

        let kem1024 = ml_kem_params(MlKemParameterSet::MlKem1024);
        assert_eq!(kem1024.name, "ML-KEM-1024");
        assert_eq!(kem1024.k, 4);
        assert_eq!(kem1024.du, 11);
        assert_eq!(kem1024.dv, 5);
        assert_eq!(kem1024.public_key_bytes, 1568);
        assert_eq!(kem1024.secret_key_bytes, 3168);
        assert_eq!(kem1024.ciphertext_bytes, 1568);
    }

    #[test]
    fn ml_dsa_parameter_tables_match_fips_names_and_sizes() {
        let dsa44 = ml_dsa_params(MlDsaParameterSet::MlDsa44);
        assert_eq!(dsa44.name, "ML-DSA-44");
        assert_eq!(dsa44.n, 256);
        assert_eq!(dsa44.q, 8_380_417);
        assert_eq!(dsa44.zeta, 1753);
        assert_eq!(dsa44.d, 13);
        assert_eq!(dsa44.tau, 39);
        assert_eq!(dsa44.lambda_bits, 128);
        assert_eq!(dsa44.gamma1, 131_072);
        assert_eq!(dsa44.gamma2, 95_232);
        assert_eq!(dsa44.k, 4);
        assert_eq!(dsa44.l, 4);
        assert_eq!(dsa44.eta, 2);
        assert_eq!(dsa44.beta, 78);
        assert_eq!(dsa44.omega, 80);
        assert_eq!(dsa44.challenge_entropy_bits, 192);
        assert_eq!(dsa44.security_category, 2);
        assert_eq!(dsa44.public_key_bytes, 1312);
        assert_eq!(dsa44.secret_key_bytes, 2560);
        assert_eq!(dsa44.signature_bytes, 2420);

        let dsa65 = ml_dsa_params(MlDsaParameterSet::MlDsa65);
        assert_eq!(dsa65.name, "ML-DSA-65");
        assert_eq!(dsa65.k, 6);
        assert_eq!(dsa65.l, 5);
        assert_eq!(dsa65.eta, 4);
        assert_eq!(dsa65.public_key_bytes, 1952);
        assert_eq!(dsa65.secret_key_bytes, 4032);
        assert_eq!(dsa65.signature_bytes, 3309);

        let dsa87 = ml_dsa_params(MlDsaParameterSet::MlDsa87);
        assert_eq!(dsa87.name, "ML-DSA-87");
        assert_eq!(dsa87.k, 8);
        assert_eq!(dsa87.l, 7);
        assert_eq!(dsa87.eta, 2);
        assert_eq!(dsa87.public_key_bytes, 2592);
        assert_eq!(dsa87.secret_key_bytes, 4896);
        assert_eq!(dsa87.signature_bytes, 4627);
    }

    #[test]
    fn shared_parameter_fixture_links_the_same_standard_names() {
        for name in [
            "ML-KEM-512",
            "ML-KEM-768",
            "ML-KEM-1024",
            "ML-DSA-44",
            "ML-DSA-65",
            "ML-DSA-87",
        ] {
            assert!(PARAMETER_FIXTURE.contains(name));
        }
        assert!(!PARAMETER_FIXTURE.contains("Kyber512"));
        assert!(!PARAMETER_FIXTURE.contains("Dilithium2"));
    }
}

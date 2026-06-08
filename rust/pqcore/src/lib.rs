#![forbid(unsafe_code)]
#![allow(dead_code)]
#![doc = include_str!("../README.md")]

mod field;
mod params;
mod polynomial;
pub mod vector_runner;

const PRODUCTION_READY: bool = false;

#[cfg(test)]
mod tests {
    use super::PRODUCTION_READY;

    #[test]
    fn crate_does_not_claim_production_readiness() {
        assert!(!PRODUCTION_READY);
    }
}

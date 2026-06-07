use core::array::from_fn;

use crate::field::ModQ;

#[derive(Clone, Debug, Eq, PartialEq)]
struct Polynomial<const N: usize, const Q: i32> {
    coefficients: [ModQ<Q>; N],
}

impl<const N: usize, const Q: i32> Polynomial<N, Q> {
    fn zero() -> Self {
        Self {
            coefficients: [ModQ::<Q>::default(); N],
        }
    }

    fn one() -> Self {
        let mut output = Self::zero();
        output.coefficients[0] = ModQ::<Q>::new(1);
        output
    }

    fn from_i32(coefficients: [i32; N]) -> Self {
        Self {
            coefficients: coefficients.map(|value| ModQ::<Q>::new(i64::from(value))),
        }
    }

    fn coeff_at(&self, index: usize) -> ModQ<Q> {
        self.coefficients[index]
    }

    fn add(&self, rhs: &Self) -> Self {
        Self {
            coefficients: from_fn(|index| self.coefficients[index] + rhs.coefficients[index]),
        }
    }

    fn sub(&self, rhs: &Self) -> Self {
        Self {
            coefficients: from_fn(|index| self.coefficients[index] - rhs.coefficients[index]),
        }
    }

    fn schoolbook_mul_negacyclic(&self, rhs: &Self) -> Self {
        let mut output = Self::zero();
        for i in 0..N {
            for j in 0..N {
                let term = self.coefficients[i] * rhs.coefficients[j];
                let degree = i + j;
                if degree < N {
                    output.coefficients[degree] += term;
                } else {
                    output.coefficients[degree - N] -= term;
                }
            }
        }
        output
    }
}

type MlKemPolynomial = Polynomial<256, 3329>;
type MlDsaPolynomial = Polynomial<256, 8_380_417>;

#[cfg(test)]
mod tests {
    use super::*;

    const POLYNOMIAL_FIXTURE: &str = include_str!("../../../fixtures/polynomial-examples.json");

    fn coefficients<const N: usize, const Q: i32>(polynomial: &Polynomial<N, Q>) -> [i32; N] {
        polynomial.coefficients.map(|value| value.value())
    }

    #[test]
    fn zero_and_construction_normalize_coefficients() {
        let zero = Polynomial::<4, 17>::zero();
        assert_eq!(coefficients(&zero), [0, 0, 0, 0]);

        let polynomial = Polynomial::<4, 17>::from_i32([17, -1, 18, -18]);
        assert_eq!(coefficients(&polynomial), [0, 16, 1, 16]);
    }

    #[test]
    fn addition_and_subtraction_are_coefficient_wise() {
        let lhs = Polynomial::<4, 17>::from_i32([1, 2, 3, 4]);
        let rhs = Polynomial::<4, 17>::from_i32([4, 3, 2, 1]);

        assert_eq!(coefficients(&lhs.add(&rhs)), [5, 5, 5, 5]);
        assert_eq!(coefficients(&lhs.sub(&rhs)), [14, 16, 1, 3]);
    }

    #[test]
    fn negacyclic_multiplication_wraps_x_to_the_n_as_minus_one() {
        let x_cubed = Polynomial::<4, 17>::from_i32([0, 0, 0, 1]);
        let x = Polynomial::<4, 17>::from_i32([0, 1, 0, 0]);
        let product = x_cubed.schoolbook_mul_negacyclic(&x);

        assert_eq!(coefficients(&product), [16, 0, 0, 0]);
    }

    #[test]
    fn multiplication_by_zero_and_one_follow_ring_identities() {
        let sample = Polynomial::<4, 17>::from_i32([5, 6, 7, 8]);
        assert_eq!(
            coefficients(&sample.schoolbook_mul_negacyclic(&Polynomial::zero())),
            [0; 4]
        );
        assert_eq!(
            coefficients(&sample.schoolbook_mul_negacyclic(&Polynomial::one())),
            coefficients(&sample)
        );
    }

    #[test]
    fn ml_kem_real_ring_wraps_degree_256_terms() {
        let mut lhs_coefficients = [0; 256];
        lhs_coefficients[255] = 1;
        let mut rhs_coefficients = [0; 256];
        rhs_coefficients[1] = 1;

        let product = MlKemPolynomial::from_i32(lhs_coefficients)
            .schoolbook_mul_negacyclic(&MlKemPolynomial::from_i32(rhs_coefficients));

        assert_eq!(product.coeff_at(0).value(), 3328);
        assert_eq!(product.coeff_at(1).value(), 0);
    }

    #[test]
    fn ml_kem_and_ml_dsa_polynomial_aliases_use_distinct_moduli() {
        let kem = MlKemPolynomial::from_i32([3329; 256]);
        let dsa = MlDsaPolynomial::from_i32([8_380_417; 256]);
        assert_eq!(kem.coeff_at(0).value(), 0);
        assert_eq!(dsa.coeff_at(0).value(), 0);
    }

    #[test]
    fn shared_polynomial_fixture_records_the_same_derivation_examples() {
        for name in [
            "tiny_x3_times_x_wraps_to_minus_one",
            "tiny_addition_is_coefficient_wise",
            "tiny_subtraction_wraps_below_zero",
            "ml_kem_x255_times_x_wraps_to_minus_one",
        ] {
            assert!(POLYNOMIAL_FIXTURE.contains(name));
        }
    }
}

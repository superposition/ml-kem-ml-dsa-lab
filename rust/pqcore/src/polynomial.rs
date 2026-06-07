use crate::field::ModQ;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Polynomial<const N: usize, const Q: i32> {
    coeffs: [ModQ<Q>; N],
}

impl<const N: usize, const Q: i32> Polynomial<N, Q> {
    pub fn zero() -> Self {
        Self {
            coeffs: [ModQ::<Q>::zero(); N],
        }
    }

    pub fn from_i32(coeffs: [i32; N]) -> Self {
        Self {
            coeffs: coeffs.map(|coefficient| ModQ::<Q>::new(i64::from(coefficient))),
        }
    }

    pub const fn coeffs(&self) -> &[ModQ<Q>; N] {
        &self.coeffs
    }

    pub fn coeff_at(&self, index: usize) -> Option<ModQ<Q>> {
        self.coeffs.get(index).copied()
    }

    pub fn add(&self, rhs: &Self) -> Self {
        Self {
            coeffs: core::array::from_fn(|index| self.coeffs[index] + rhs.coeffs[index]),
        }
    }

    pub fn sub(&self, rhs: &Self) -> Self {
        Self {
            coeffs: core::array::from_fn(|index| self.coeffs[index] - rhs.coeffs[index]),
        }
    }

    pub fn schoolbook_mul_negacyclic(&self, rhs: &Self) -> Self {
        let mut result = Self::zero();

        for i in 0..N {
            for j in 0..N {
                let term = self.coeffs[i] * rhs.coeffs[j];
                let degree = i + j;
                if degree < N {
                    result.coeffs[degree] += term;
                } else {
                    result.coeffs[degree - N] -= term;
                }
            }
        }

        result
    }
}

#[cfg(test)]
mod tests {
    use super::Polynomial;

    #[test]
    fn adds_and_subtracts_coefficients() {
        let a = Polynomial::<4, 17>::from_i32([1, 2, 3, 4]);
        let b = Polynomial::<4, 17>::from_i32([4, 3, 2, 1]);

        assert_eq!(
            a.add(&b)
                .coeffs()
                .iter()
                .map(|value| value.value())
                .collect::<Vec<_>>(),
            vec![5, 5, 5, 5]
        );
        assert_eq!(
            a.sub(&b)
                .coeffs()
                .iter()
                .map(|value| value.value())
                .collect::<Vec<_>>(),
            vec![14, 16, 1, 3]
        );
    }

    #[test]
    fn multiplies_in_negacyclic_ring() {
        let x3 = Polynomial::<4, 17>::from_i32([0, 0, 0, 1]);
        let x = Polynomial::<4, 17>::from_i32([0, 1, 0, 0]);
        let product = x3.schoolbook_mul_negacyclic(&x);

        assert_eq!(
            product
                .coeffs()
                .iter()
                .map(|value| value.value())
                .collect::<Vec<_>>(),
            vec![16, 0, 0, 0]
        );
    }
}

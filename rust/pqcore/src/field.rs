use core::ops::{Add, AddAssign, Mul, Neg, Sub, SubAssign};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub(crate) struct ModQ<const Q: i32> {
    value: i32,
}

impl<const Q: i32> ModQ<Q> {
    pub(crate) fn new(value: i64) -> Self {
        assert!(Q > 0, "modulus must be positive");
        let q = i64::from(Q);
        let mut reduced = value % q;
        if reduced < 0 {
            reduced += q;
        }
        Self {
            value: i32::try_from(reduced).expect("reduced value fits i32"),
        }
    }

    pub(crate) fn value(self) -> i32 {
        self.value
    }

    pub(crate) fn centered(self) -> i32 {
        if self.value > Q / 2 {
            self.value - Q
        } else {
            self.value
        }
    }
}

impl<const Q: i32> Default for ModQ<Q> {
    fn default() -> Self {
        Self { value: 0 }
    }
}

impl<const Q: i32> Add for ModQ<Q> {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self::new(i64::from(self.value) + i64::from(rhs.value))
    }
}

impl<const Q: i32> AddAssign for ModQ<Q> {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}

impl<const Q: i32> Sub for ModQ<Q> {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        Self::new(i64::from(self.value) - i64::from(rhs.value))
    }
}

impl<const Q: i32> SubAssign for ModQ<Q> {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}

impl<const Q: i32> Mul for ModQ<Q> {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        Self::new(i64::from(self.value) * i64::from(rhs.value))
    }
}

impl<const Q: i32> Neg for ModQ<Q> {
    type Output = Self;

    fn neg(self) -> Self::Output {
        Self::new(-i64::from(self.value))
    }
}

type MlKemField = ModQ<3329>;
type MlDsaField = ModQ<8_380_417>;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn canonical_representation_normalizes_boundary_values() {
        assert_eq!(ModQ::<17>::new(0).value(), 0);
        assert_eq!(ModQ::<17>::new(17).value(), 0);
        assert_eq!(ModQ::<17>::new(18).value(), 1);
        assert_eq!(ModQ::<17>::new(-1).value(), 16);
        assert_eq!(ModQ::<17>::new(-18).value(), 16);
    }

    #[test]
    fn centered_representation_preserves_stored_value() {
        let positive = ModQ::<17>::new(8);
        let negative = ModQ::<17>::new(9);

        assert_eq!(positive.centered(), 8);
        assert_eq!(positive.value(), 8);
        assert_eq!(negative.centered(), -8);
        assert_eq!(negative.value(), 9);
    }

    #[test]
    fn arithmetic_closes_modulo_q() {
        assert_eq!((ModQ::<17>::new(16) + ModQ::<17>::new(2)).value(), 1);
        assert_eq!((ModQ::<17>::new(1) - ModQ::<17>::new(4)).value(), 14);
        assert_eq!((ModQ::<17>::new(16) * ModQ::<17>::new(16)).value(), 1);
        assert_eq!((-ModQ::<17>::new(1)).value(), 16);
    }

    #[test]
    fn ml_kem_and_ml_dsa_moduli_are_distinct() {
        assert_eq!(MlKemField::new(3329).value(), 0);
        assert_eq!(MlKemField::new(-1).value(), 3328);
        assert_eq!(MlDsaField::new(8_380_417).value(), 0);
        assert_eq!(MlDsaField::new(-1).value(), 8_380_416);
    }

    #[test]
    fn ml_dsa_multiplication_uses_wide_intermediate() {
        let value = MlDsaField::new(8_380_416) * MlDsaField::new(8_380_416);
        assert_eq!(value.value(), 1);
    }
}

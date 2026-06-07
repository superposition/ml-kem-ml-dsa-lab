use core::fmt;
use core::ops::{Add, AddAssign, Mul, Neg, Sub, SubAssign};

#[derive(Copy, Clone, PartialEq, Eq, Default)]
pub struct ModQ<const Q: i32> {
    value: i32,
}

impl<const Q: i32> ModQ<Q> {
    pub const fn zero() -> Self {
        Self { value: 0 }
    }

    pub fn new(value: i64) -> Self {
        assert!(Q > 0, "modulus must be positive");
        let mut reduced = value % i64::from(Q);
        if reduced < 0 {
            reduced += i64::from(Q);
        }
        Self {
            value: reduced as i32,
        }
    }

    pub const fn value(self) -> i32 {
        self.value
    }

    pub const fn centered(self) -> i32 {
        if self.value > Q / 2 {
            self.value - Q
        } else {
            self.value
        }
    }
}

impl<const Q: i32> fmt::Debug for ModQ<Q> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_tuple("ModQ").field(&self.value).finish()
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

#[cfg(test)]
mod tests {
    use super::ModQ;

    #[test]
    fn normalizes_positive_and_negative_values() {
        assert_eq!(ModQ::<3329>::new(3330).value(), 1);
        assert_eq!(ModQ::<3329>::new(-1).value(), 3328);
        assert_eq!(ModQ::<8380417>::new(-8380418).value(), 8380416);
    }

    #[test]
    fn computes_modular_arithmetic() {
        let a = ModQ::<17>::new(15);
        let b = ModQ::<17>::new(5);

        assert_eq!((a + b).value(), 3);
        assert_eq!((a - b).value(), 10);
        assert_eq!((a * b).value(), 7);
        assert_eq!((-b).value(), 12);
    }

    #[test]
    fn exposes_centered_representatives() {
        assert_eq!(ModQ::<17>::new(8).centered(), 8);
        assert_eq!(ModQ::<17>::new(9).centered(), -8);
    }
}

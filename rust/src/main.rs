fn main() {
    println!("Hello, world!");
    println!("res = {}", roman_to_int("MCMXCIV".to_string()));
}

// impl Solution {
pub fn single_roman_to_int(c: char) -> i32 {
    match c {
        'I' => 1,
        'V' => 5,
        'X' => 10,
        'L' => 50,
        'C' => 100,
        'D' => 500,
        'M' => 1000,
        _ => panic!("Ain't special"),
    }
}

pub fn roman_to_int(s: String) -> i32 {
    let mut res = 0;
    if s.is_empty() {
        return res;
    }

    let mut cur;
    let mut prev = 1001;

    for c in s.chars() {
        cur = single_roman_to_int(c);

        if cur > prev {
            res += cur - prev - prev;
        } else {
            res += cur;
        }
        prev = cur
    }
    res
}

pub fn int_to_roman(num: i32) -> String {}

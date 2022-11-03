#![allow(dead_code)]

use std::ops::Sub;
mod lru_cache;




fn main() {
    println!("Hello, world!");
    println!("res = {}", roman_to_int("MCMXCIV".to_string()));

    println!("res = {}", int_to_roman(1994));
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

pub fn int_to_roman(num: i32) -> String {
    let mut res = String::new();

    let helper = [
        (1, "I"),
        (4, "IV"),
        (5, "V"),
        (9, "IX"),
        (10, "X"),
        (40, "XL"),
        (50, "L"),
        (90, "XC"),
        (100, "C"),
        (400, "CD"),
        (500, "D"),
        (900, "CM"),
        (1000, "M"),
    ];

    let mut remainder = num;

    for (decimal, roman) in helper.iter().rev() {
        if remainder > *decimal {
            let mut count = remainder / decimal;
            remainder = remainder % decimal;
            while count > 0 {
                res.push_str(roman);
                count = count.sub(1);
            }
        } else {
            continue;
        }
    }

    res
}

pub fn int_to_roman2(num: i32) -> String {
    let num_str = num.to_string();

    let mut romans: Vec<&str> = num_str
        .chars()
        .rev()
        .enumerate()
        .map(|(idx, single_char)| {
            let char_num = single_char.to_digit(10).unwrap();

            match (idx, char_num) {
                (0, 1) => "I",
                (0, 2) => "II",
                (0, 3) => "III",
                (0, 4) => "IV",
                (0, 5) => "V",
                (0, 6) => "VI",
                (0, 7) => "VII",
                (0, 8) => "VIII",
                (0, 9) => "IX",

                (1, 1) => "X",
                (1, 2) => "XX",
                (1, 3) => "XXX",
                (1, 4) => "XL",
                (1, 5) => "L",
                (1, 6) => "LX",
                (1, 7) => "LXX",
                (1, 8) => "LXXX",
                (1, 9) => "XC",

                (2, 1) => "C",
                (2, 2) => "CC",
                (2, 3) => "CCC",
                (2, 4) => "CD",
                (2, 5) => "D",
                (2, 6) => "DC",
                (2, 7) => "DCC",
                (2, 8) => "DCCC",
                (2, 9) => "CM",

                (3, 1) => "M",
                (3, 2) => "MM",
                (3, 3) => "MMM",
                _ => "",
            }
        })
        .collect();

    romans.reverse();
    romans.join("")
}

pub fn array_strings_are_equal(word1: Vec<String>, word2: Vec<String>) -> bool {
    let word1 = word1
        .into_iter()
        .map(|x| x.into_bytes())
        .flatten()
        .collect::<Vec<_>>();
    let word2 = word2
        .into_iter()
        .map(|x| x.into_bytes())
        .flatten()
        .collect::<Vec<_>>();

    word1 == word2
}

#[derive(PartialEq, Eq, Clone, Debug)]
pub struct ListNode {
    pub val: i32,
    pub next: Option<Box<ListNode>>,
}

impl ListNode {
    #[inline]
    fn new(val: i32) -> Self {
        ListNode { next: None, val }
    }
}

pub fn add_two_numbers1(
    l1: Option<Box<ListNode>>,
    l2: Option<Box<ListNode>>,
) -> Option<Box<ListNode>> {

    let mut res_dummy = Some(Box::new(ListNode::new(0)));
    let mut res_head = res_dummy.as_mut().unwrap();


    let mut carry: bool = false;

    let mut head1 = l1.as_ref();
    let mut head2 = l2.as_ref();

    while head1.is_some() || head2.is_some() {
        let first: i32;
        let second: i32;

        res_head.next = Some(Box::new(ListNode::new(0)));

        res_head = res_head.next.as_mut().unwrap();

        if let Some(a) = head1 {
            first = a.val
        } else {
            first = 0;
        }

        if let Some(b) = head2 {
            second = b.val;
        } else {
            second = 0;
        }

        let mut res = first + second;

        if carry {
            res += 1;
        }

        carry = res >= 10;

        res_head.val = res % 10;

        if head1.is_some() {
            head1 = head1.unwrap().next.as_ref()
        }

        if head2.is_some() {
            head2 = head2.unwrap().next.as_ref()
        }
    }

    if carry == true {
        res_head.next = Some(Box::new(ListNode::new(1)));
    }

    res_dummy.unwrap().next
}


fn add_two_numbers2(
    l1: Option<Box<ListNode>>,
    l2: Option<Box<ListNode>>,
) -> Option<Box<ListNode>> {

    match (l1, l2) {
        (None, None) => None,
        (Some(node), None) | (None, Some(node)) => Some(node),
        (Some(node1), Some(node2)) => {

            let sum = node1.val + node2.val;

            let (_, _) = if sum < 10 {
                (false, sum)
            } else {
                (true, sum % 10)
            };

            // Some(Box::new)


            None
        }
    }
}

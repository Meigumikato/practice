#![allow(dead_code, unused)]

use std::collections::HashMap;
use std::collections::VecDeque;






struct LRUCache1 {
    size : usize,
    fresh_list : VecDeque<i32>,
    hash_table : HashMap<i32, i32>,
}


/** 
 * `&self` means the method takes an immutable reference.
 * If you need a mutable reference, change it to `&mut self` instead.
 */
impl LRUCache1 {

    fn new(capacity: i32) -> Self {

        Self { 
            size: capacity as usize, 
            fresh_list: VecDeque::new(), 
            hash_table: HashMap::new() 
        }
    }

    
    fn get(&mut self, key: i32) -> i32 {
        if self.hash_table.contains_key(&key) {

            let pos = self.fresh_list.iter().position(|x| { x == &key}).unwrap();
            self.fresh_list.remove(pos);
            self.fresh_list.push_front(key);
            self.hash_table.get(&key).cloned().unwrap()
        } else {
            -1
        }
    }
    
    fn put(&mut self, key: i32, value: i32) {

        if self.hash_table.contains_key(&key) {
            let pos = self.fresh_list.iter().position(|x| { x == &key}).unwrap();
            self.fresh_list.remove(pos);
            self.fresh_list.push_front(key);
            self.hash_table.insert(key, value);
        } else {
            if self.fresh_list.len() == self.size {
                let key = self.fresh_list.pop_back().unwrap();
                self.hash_table.remove(&key);
            }

            self.fresh_list.push_front(key);
            self.hash_table.insert(key, value);
        }
    }
}

#[derive(Debug)]
struct Node<T : Copy> {
    val : T,
    prev : *mut Node<T>,
    next : *mut Node<T>,
}


impl<T: Copy> Node<T> {

    fn new(val: T) -> Self {
        Self { 
            val, 
            prev: std::ptr::null_mut(), 
            next: std::ptr::null_mut() 
        }
    }
    
}


#[derive(Debug)]
struct ListNode<T : Copy> {
    size : usize,
    head : *mut Node<T>,
    tail : *mut Node<T>,
}


impl<T: Copy + std::cmp::PartialEq> ListNode<T> {

    fn new() -> Self {
        Self { 
            size: 0, 
            head: std::ptr::null_mut(),
            tail: std::ptr::null_mut(),
        }
    }

    fn begin(&self) -> *mut Node<T> {
       self.head 
    }

    fn move_head(&mut self, node : *mut Node<T>) {
        unsafe {

            if node != self.head {

                let w_prev = (*node).prev;
                let w_next = (*node).next;

                if node == self.tail {
                    self.tail = w_prev;
                }

                if !w_prev.is_null() {
                    (*w_prev).next = w_next;
                }

                if !w_next.is_null() {
                    (*w_next).prev = w_prev;
                }


                (*node).next = self.head;
                (*self.head).prev = node;
                (*node).prev = std::ptr::null_mut();

                self.head = node;
            }
        }
    }

    fn push_front(&mut self, val : T) {
        let node = Box::leak(Box::new(Node::new(val)));
        if self.head.is_null() {
            self.head = node;
            self.tail = node;

        } else {
            unsafe {
                (*self.head).prev = node;
                (*node).next = self.head;
                self.head = node;
            }
        }

        self.size += 1;
    }

    fn pop_back(&mut self) -> T {
        unsafe {

            let pop_val = (*self.tail).val;
            let t_prev = (*self.tail).prev;

            if t_prev.is_null() {
                Box::from_raw(self.tail);
                self.tail = std::ptr::null_mut();
                self.head = std::ptr::null_mut();
            } else {

                (*self.tail).prev = std::ptr::null_mut();

                (*t_prev).next = std::ptr::null_mut();

                Box::from_raw(self.tail);
                self.tail = t_prev;
            }

            self.size -= 1;

            return pop_val;
        }
    }
}

struct LRUCache {
    size : usize,
    fresh_list : ListNode<(i32, i32)>,
    hash_table : HashMap<i32, *mut Node<(i32, i32)>>,
}


/** 
 * `&self` means the method takes an immutable reference.
 * If you need a mutable reference, change it to `&mut self` instead.
 */
impl LRUCache {

    fn new(capacity: i32) -> Self {

        Self { 
            size: capacity as usize, 
            fresh_list: ListNode::new(), 
            hash_table: HashMap::new() 
        }
    }

    
    fn get(&mut self, key: i32) -> i32 {
        if self.hash_table.contains_key(&key) {
            let val = self.hash_table.get(&key).cloned().unwrap();
            self.fresh_list.move_head(val);
            unsafe { (*val).val.1 }
        } else {
            -1
        }
    }
    
    fn put(&mut self, key: i32, value: i32) {

        if self.hash_table.contains_key(&key) {
            let val = self.hash_table.get(&key).cloned().unwrap();
            self.fresh_list.move_head(val);
            unsafe { (*val).val.1 = value };

            // self.hash_table.insert(key, self.fresh_list.head);
        } else {
            if self.fresh_list.size == self.size {
                let poped = self.fresh_list.pop_back();
                self.hash_table.remove(&poped.0);
            }

            self.fresh_list.push_front((key, value));
            self.hash_table.insert(key, self.fresh_list.begin());
        }
    }
}


#[cfg(test)]
mod test {
    use super::*;
    
    #[test]
    fn test() {
        let mut cache = LRUCache::new(2);
        cache.put(1, 1);
        cache.put(2, 2);
        assert_eq!(cache.get(1), 1);
        cache.put(3, 3);
        assert_eq!(cache.get(2), -1);
        cache.put(4, 4);
        assert_eq!(cache.get(1), -1);
        assert_eq!(cache.get(3), 3);
        assert_eq!(cache.get(4), 4);
    }

    #[test]
    fn a_test() {
        let mut cache = LRUCache::new(2);
        cache.put(1, 0);
        cache.put(2, 2);

        assert_eq!(cache.get(1), 0);

        cache.put(3, 3);
        assert_eq!(cache.get(2), -1);
        cache.put(4, 4);

        assert_eq!(cache.get(1), -1);
        assert_eq!(cache.get(3), 3);
        assert_eq!(cache.get(4), 4);
    }
}

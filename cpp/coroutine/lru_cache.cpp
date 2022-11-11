#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <list>
#include <cassert>

class LRUCache {
public:
    LRUCache(int capacity) : capacity_(capacity) {

    }
    
    int get(int key) {

      int res = -1;
      if (hash_.count(key) > 0) {

        list_.splice(list_.begin(), list_, hash_[key]);
        res = hash_[key]->second;
      }

      // PrintDebug();
      return res;
    }
    
    void put(int key, int value) {
      if (hash_.count(key)) {
        hash_[key]->second = value;
        list_.splice(list_.begin(), list_, hash_[key]);
      } else {

        if (list_.size() + 1 > capacity_) {
          auto back = list_.back();
          hash_.erase(hash_.find(back.first));
          list_.pop_back();
        }

        list_.push_front({key, value});
        hash_[key] = list_.begin();
      }

      // PrintDebug();
    }

    void PrintDebug() {
      std::for_each(list_.begin(), list_.end(), [](auto& x) {
                      std::cout << x.first << " ";
                    });
      std::cout << "\n";
    }

private:
  int capacity_;
  std::list<std::pair<int, int>> list_;
  std::unordered_map<int, std::list<std::pair<int, int>>::iterator> hash_;
};


int main() {

  LRUCache LRUCache{2};

  LRUCache.put(1, 1); // cache is {1=1}
  LRUCache.put(2, 2); // cache is {1=1, 2=2}
  assert(LRUCache.get(1) == 1);    // return 1

  LRUCache.put(3, 3); // LRU key was 2, evicts key 2, cache is {1=1, 3=3}
  assert(LRUCache.get(2) == -1);    // returns -1 (not found)
  LRUCache.put(4, 4); // LRU key was 1, evicts key 1, cache is {4=4, 3=3}
  //
  assert(LRUCache.get(1) == -1);    // return -1 (not found)
  assert(LRUCache.get(3) == 3);    // return 3
  assert(LRUCache.get(4) == 4);    // return 4
  return 0;
}

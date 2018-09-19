#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include <cassert>

namespace utility {

#define INIT_ARRAY_POOL_SIZE    (1024)

template<typename value_t>
class ArrayPool {
public:
    ArrayPool() : valid_count_(0), max_elem_size_(INIT_ARRAY_POOL_SIZE) { }
    ArrayPool(uint32_t init_size) : valid_count_(0), max_elem_size_(init_size<65535 ? init_size : 65535) { }

    value_t* Alloc() {
        if (valid_count_ < max_elem_size_/4) Enlarge();
        // alloc from stack
        if (NoContinues()) {
            IndexT id = free_st_.top();
            free_st_.pop();
            ElemT &e = pools_[id.chunk][id.index];
            e.id = id;
            valid_count_--;
            return &e.val;
        } else {
            ElemT &e = pools_[index_.chunk][index_.index];
            e.id = index_;
            NextIndex();
            valid_count_--;
            return &e.val;
        }
    }

    void Delete(value_t* val) {
        IndexT* id = (IndexT*)((char*)val - offsetof(ElemT, val));
        free_st_.push(*id);
        valid_count_++;
    }

    size_t size() const { return pools_.size() * max_elem_size_ - valid_count_; }

    void clear() {
        free_st_.clear();
        pools_.clear();
        valid_count_ = 0;
    }

private:
    bool NoContinues() { return index_.chunk >= pools_.size(); }

    void NextIndex() {
        index_.index++;
        // move to next pool
        if ((uint32_t)index_.index == max_elem_size_) {
            index_.chunk++;
            index_.index = 0;
        }
    }

    void Enlarge() {
        pools_.push_back(std::vector<ElemT>());
        pools_.back().resize(max_elem_size_);
        valid_count_ += max_elem_size_;
    }

private:
    struct IndexT {
        uint16_t chunk;
        uint16_t index;
        IndexT() : chunk(0), index(0) {}
    };

    struct ElemT {
        IndexT  id;
        value_t val;
    };

    uint32_t valid_count_;
    uint32_t max_elem_size_;
    IndexT index_; // current index
    std::stack<IndexT> free_st_;  // free elements
    std::vector<std::vector<ElemT>> pools_; // elements pool
};

}  // namespace utility

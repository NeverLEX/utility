#pragma once
#include <iostream>
#include <vector>

namespace utility {

#define INIT_ARRAY_LIST_SIZE    (1024)

template<typename value_t>
class ArrayList {
public:
    ArrayList(int32_t init_size) { Init(init_size); }

    void Init(int32_t init_size) {
        init_size = init_size < INIT_ARRAY_LIST_SIZE ? INIT_ARRAY_LIST_SIZE : (1 << (GetHighestOrderBit(init_size) + 1));
        used_.resize(init_size, 0); // set additional elements as zero
        elems_.resize(init_size);
        total_count_ = init_size - 1;
        valid_count_ = init_size - 1;  // valid_count_ will always be 2^n - 1
                                       // first item means end of list
        used_[0] = 1;
        cur_index_ = 1;
    }

    value_t& AllocHead(int32_t& head_id) {
        if (valid_count_ < elems_.size() / 8) Enlarge();
        do {
            if (!used_[cur_index_]) {
                // set used_ and next status
                used_[cur_index_] = 1;
                Elem &e = elems_[cur_index_];
                e.next  = head_id;
                head_id = cur_index_;
                // add cur_index_ and reduce valid_count_
                NextCurIndex();
                valid_count_--;
                return e.val;
            } else {
                NextCurIndex();
            }
        } while(true);
    }

    void DeleteLinks(int32_t &head_id) {
        while (head_id) {
            used_[head_id] = 0;  // free elem
            head_id = elems_[head_id].next;
        }
    }

    value_t& GetValue(int id) {
        if (id<0 || id>elems_.size() || !used_[id]) return elems_[0].val;
        return elems_[id].val;
    }

    const value_t& GetValue(int id) const {
        if (id<0 || id>elems_.size() || !used_[id]) return elems_[0].val;
        return elems_[id].val;
    }

    const int32_t GetNextId(int32_t id) const {
        if (id<0 || id>elems_.size() || !used_[id]) return 0;
        return elems_[id].next;
    }

    void clear() {
        const int32_t size = (int32_t)used_.size();
        memset(&used_[0], 0, size*sizeof(used_[0]));
        memset(&elems_[0], 0, size*sizeof(elems_[0]));
        total_count_ = size - 1;
        valid_count_ = size - 1;  // valid_count_ will always be 2^n - 1
                                  // first item means end of list
        used_[0] = 1;
        cur_index_ = 1;
    }

private:
    ArrayList() { } // disable

    struct Elem {
        value_t val;
        int32_t next;
    };

    void NextCurIndex() { cur_index_ = (cur_index_ + 1) & total_count_; }

    void Enlarge() {
        const int32_t size = (int32_t)elems_.size();
        used_.resize(size<<1, 0); // set additional elements as zero
        elems_.resize(size<<1);
        total_count_ += size;
        valid_count_ += size;
        cur_index_ = size;
    }

    int32_t GetHighestOrderBit(int32_t val) {
        if (val == 0) return 0;
        int ret = 1;
        while (val >>= 1) ret++;
        return ret;
    }

private:
    int32_t total_count_;
    int32_t valid_count_;
    int32_t cur_index_;
    std::vector<int8_t>  used_;
    std::vector<Elem>    elems_;
};

}  // namespace utility

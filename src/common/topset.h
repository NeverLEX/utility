#pragma once
#include <iostream>

namespace utility {

template<typename key_t, typename value_t, const int TOPSET_MAX_COUNT, const int MAX = 1>
class topset {
public:
    topset() : count_(0) {}

    void Insert(const key_t& key, const value_t& val) {
        if (count_ < TOPSET_MAX_COUNT) {
            // insert new position = count
            int32_t parent_pos = (++count_)>>1;
            Elem *new_pos = &elems_[count_], *parent = &elems_[parent_pos];
            while (parent_pos && IsBetter(parent->key, key)) {
                *new_pos = *parent;
                new_pos = parent;
                parent = &elems_[parent_pos >>= 1];
            }
            new_pos->key = key;
            new_pos->val = val;
        } else if (TOPSET_MAX_COUNT != 0 && IsBetter(key, elems_[1].key)){
            Replace(key, val);
        }
    }

    struct Elem {
        key_t   key;
        value_t val;
    };

    const Elem* Sort() {
        if (count_ <= 1) return &elems_[count_];
        const int32_t heap_count = count_;
        Elem e = elems_[count_];
        elems_[count_--] = elems_[1];
        while (count_) {
            Replace(e.key, e.val);
            e = elems_[count_];
            elems_[count_--] = elems_[1];
        }
        count_ = heap_count;
        // swap
        Elem *rr = &elems_[1], *ww = rr + count_ - 1;
        while (rr < ww) {
            e = *rr;
            *rr++ = *ww;
            *ww-- = e;
        }
        return &elems_[1];
    }

    int32_t size() const { return count_; }

private:
    inline bool IsBetter(const key_t &a, const key_t &b) { return MAX ? (a>b) : (a<b); }

    void Replace(const key_t& key, const value_t& val) {
        if (count_ == 1) {
            elems_[1].key = key;
            elems_[1].val = val;
            return;
        }
        int32_t diff = (1<<1);
        Elem *cur = &elems_[1], *e = cur + count_;
        Elem *l = &elems_[2], *r = (l+1<e) ? l+1 : l;
        Elem *x = IsBetter(l->key, r->key) ? (++diff, r) : l;
        while (IsBetter(val, x->key)) {
            *cur = *x; cur = x;
            l = x + diff; diff <<= 1;
            if (l >= e) break;
            if (l+1 == e) {
                if (IsBetter(val, l->key)) { *cur = *l; cur = l; }
                break;
            } else {
                r = l+1;
                x = IsBetter(l->key, r->key) ? (++diff, r) : l;
            }
        }
        cur->key = key;
        cur->val = val;
    }

private:

    int32_t count_;
    Elem elems_[TOPSET_MAX_COUNT + 1];  // values_[0] reserve do not use
};

} //  namespace utility

#pragma once
#include <iostream>
#include <vector>
#include <new>

namespace utility {

#define ArrayMapMemoryLimit    (sizeof(void*)==8? size_t(16ULL<<30): size_t(2UL<<30))
#define ARRAY_MAP_TABLE_SIZE   4//(4<<10) //must be power of 2

template <typename key_, typename value_t>
class ArrayMap {
    key_t * k_table; //key
    value_t * v_table; //value (default 0)
    size_t tablesize, goldensize, nodesize;

public:
    ArrayMap(size_t init_size) { Init(init_size); }
    ~ArrayMap() { Exit(); }

    bool Init(size_t init_size) {
        v_table    = nullptr;
        k_table    = nullptr;
        tablesize  = init_size < ARRAY_MAP_TABLE_SIZE ? ARRAY_MAP_TABLE_SIZE : (1 << (GetHighestOrderBit(init_size) + 1)); // tablesize = 2^n
        goldensize = (tablesize*7/10);

        v_table = new value_t[tablesize];
        k_table = new key_t[tablesize];
        if (nullptr == v_table || nullptr == k_table) return false;

        memset(v_table, 0, sizeof(value_t)*tablesize);
        memset(k_table, 0, sizeof(key_t)*tablesize);
        nodesize = 0;
        return true;
    }

    void Exit() {
        if(k_table) delete [] k_table, k_table=NULL;
        if(v_table) delete [] v_table, v_table=NULL;
        tablesize = goldensize = nodesize = 0;
    }

    void clear() {
        nodesize = 0;
        if(v_table) memset(v_table, 0, tablesize * sizeof(value_t));
        if(k_table) memset(k_table, 0, tablesize * sizeof(key_t));
    }

    size_t size() const {
        return nodesize;
    }

    value_t & Insert(key_t key) {
        if(nodesize > goldensize) Enlarge();
        size_t i = MyHash(key) & (tablesize-1);
        while(k_table[i]){
            if(IsEqualKey(k_table[i], key)) return v_table[i];
            i = (i+1) & (tablesize-1);
        }
        k_table[i] = key;
        ++nodesize;
        return v_table[i];
    }

    value_t Find(key_t key) const {
        size_t i = MyHash(key);
        do{
            i &= (tablesize-1);
            if(!k_table[i]) return 0;
            if(IsEqualKey(k_table[i], key)) return v_table[i];
            i++;
        }while(true);
    }

    value_t & operator[](key_t key) {
        return Insert(key);
    }

    //delete the (key -> value), return the value or 0 if not found.
    value_t Delete(key_t key) {
        size_t i = MyHash(key) & (tablesize-1);
        while(true){
            if(!k_table[i]) return 0; //not found
            if(IsEqualKey(k_table[i], key)) break;
            i = (i+1) & (tablesize-1);
        }
        value_t v = v_table[i];
        size_t k = i;
        while(k_table[(i = (i+1) & (tablesize-1))]){
            size_t h = MyHash(k_table[i]) & (tablesize-1);
            if((h<=k && k<i) || (i<h && h<=k) || (k<i && i<h)){
                //when `k' is in the middle of path(h->i): move [i] to [k]
                k_table[k] = k_table[i];
                v_table[k] = v_table[i];
                k = i;
            }
        }
        k_table[k] = 0;
        v_table[k] = 0;
        --nodesize;
        return v;
    }

    value_t Enum(size_t & i, key_t & key) const {
        for(; i<tablesize; i++){
            if(k_table[i]){
                key = k_table[i];
                return v_table[i++];
            }
        }
        key = 0;
        return 0;
    }

private:
    ArrayMap() : k_table(nullptr), v_table(nullptr) { } // disable

    int32_t GetHighestOrderBit(size_t val) {
        if (val == 0) return 0;
        int ret = 1;
        while (val >>= 1) ret ++;
        return ret;
    }

    bool Enlarge() {
        const size_t byMemoryLimit = ArrayMapMemoryLimit / (sizeof(key_t)+sizeof(value_t));
        if(tablesize >= byMemoryLimit){
            std::cout << "(ERROR) tablesize(" << tablesize << ") is large then MemoryLimit(" << byMemoryLimit << "), nodesize = " << nodesize << std::endl;
            return false;
        }
        size_t oldsize = tablesize, oldnodesize = nodesize;
        value_t * oldv = v_table;
        key_t * olds = k_table;
        if(!this->Init(oldsize<<1)) {
            std::cout << "(ERROR) Init(oldsize<<1) failed!" << std::endl;
            return false;
        }
        this->nodesize = oldnodesize;
        for(size_t i=0; i<oldsize; i++){
            if(olds[i]){
                size_t j = MyHash(olds[i]) & (tablesize-1);
                while(k_table[j]) j = (j+1) & (tablesize-1);
                k_table[j] = olds[i];
                v_table[j] = oldv[i];
            }
        }
        delete [] olds;
        delete [] oldv;
        return true;
    }

    static inline bool IsEqualKey(key_t a, key_t b) {
        return a == b;
    }

    // for int
    static inline size_t MyHash(key_t val) {
        return (size_t)val * 25214903917ULL + 11;
    }

    // for string
    static inline size_t sdbm_hash(const unsigned char * str) {
        size_t hash = 0;
        int c;
        while ((c = *(str++))){
            hash = c + (hash << 6) + (hash << 16) - hash; //hash = hash * 257 + c;
        }
        return hash;
    }
};

}  // namespace utility


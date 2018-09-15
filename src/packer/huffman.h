/**
 *  This is an implementation of Huffman coding.
 *  Usage:
 *  1. encode a stream buffer
 *    Huffman huffman;
 *    huffman.Encode(stream_buffer, encode_buffer);
 *
 *  2. decode a stream buffer
 *    Huffman huffman;
 *    huffman.Decode(stream_buffer, encode_buffer);
 *
 */

#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include "log.h"

namespace huffman {

class Huffman {

    // Huffman Tree Node
    struct HuffmanTree {
        char ch;
        int freq; // frequency of ch
        HuffmanTree *left;
        HuffmanTree *right;

        // constructed functions
        HuffmanTree(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
        HuffmanTree(char ch, int freq, HuffmanTree *left, HuffmanTree *right) : ch(ch), freq(freq), left(left), right(right) {}

        // free left child and right child
        ~HuffmanTree() {
            if (nullptr != left) delete left, left = nullptr;
            if (nullptr != right) delete right, right = nullptr;
        }
    };

    // Compare two tree nodes
    class CompareTree {
        public:
            bool operator()(HuffmanTree *a, HuffmanTree *b) {
                return a->freq == b->freq ? ((uint8_t)a->ch > (uint8_t)b->ch) : (a->freq > b->freq);
            }
    };

    typedef std::vector<bool> huffman_code_t;
    typedef std::map<char, huffman_code_t> huffman_code_table_t;

public:
    Huffman() : root_(nullptr) {}
    ~Huffman() { if (nullptr != root_) delete root_, root_ = nullptr; }

    /** @brief encode a stream buffer use huffman
     *  @param stream_buffer input stream buffer
     *  @param encode_buffer outout encode buffer, code table | int(encode_bit_size) | encode buffer
     *  @return success or fail
     */
    bool Encode(const std::vector<char> &stream_buffer, std::vector<char> &encode_buffer);

    /** @brief decode a compressed buffer use huffman
     *  @param encode_buffer input encode buffer
     *  @param stream_buffer outout stream buffer
     *  @return success or fail
     */
    template<typename streambuf_t>
    bool Decode(const std::vector<char> &encode_buffer, streambuf_t &stream_buffer);

private:
    /** @brief build frequency map
     *  @param stream_buffer input stream buffer
     *  @return success or fail
     */
    bool BuildFreqMap(const std::vector<char> &stream_buffer);

    /** @brief write frequency map to compressed buffer
     *  @param encode_buffer output encode buffer
     *  @return success or fail
     */
    bool WriteFreqMap(std::vector<char> &encode_buffer);

    /** @brief read frequency map from compressed buffer
     *  @param encode_buffer input encode buffer
     *  @param table_size freq table size
     *  @return success or fail
     */
    bool ReadFreqMap(const std::vector<char> &encode_buffer, size_t &table_size);

    /** @brief build a Huffman Tree
     *  @return success or fail
     */
    bool BuildTree();

    /** @brief build code table
     *  @param htree huffman tree
     */
    bool BuildCodeTable();

private:
    HuffmanTree *root_;
    huffman_code_table_t code_table_;
    std::map<char, int> char_freq_map_;
};

/** @brief decode a compressed buffer use huffman
 *  @param encode_buffer input encode buffer
 *  @param stream_buffer outout stream buffer
 *  @return success or fail
 */
template<typename streambuf_t>
bool Huffman::Decode(const std::vector<char> &encode_buffer, streambuf_t &stream_buffer) {
    size_t table_size = 0;
    stream_buffer.clear();
    if (!ReadFreqMap(encode_buffer, table_size)) {
        LOG_ERR << "read encode buffer error.";
        return false;
    }

    if (!BuildTree()) {
        LOG_ERR << "build huffman tree error.";
        return false;
    }

    // empty buffer
    if (nullptr == root_) return true;
    if (table_size + sizeof(uint64_t) > encode_buffer.size()) {
        LOG_ERR << "encode buffer size error.";
        return false;
    }

    const char *pencode = &encode_buffer[table_size];
    const uint64_t encode_bit_size = *(uint64_t*)pencode; pencode += sizeof(uint64_t);
    if (encode_bit_size == 0) return true;

    uint64_t encode_8_bit_size = encode_bit_size/8*8;
    uint64_t encode_remain_bit_size = encode_bit_size - encode_8_bit_size;
    stream_buffer.reserve(encode_bit_size/8);
    const HuffmanTree *node = root_;

    // 8bit chunk, encode stream + align (zero) > 8bit
    // only for performance
    {
        #define LOOP_INNER_(i) \
            node = (ch & (1<<i)) ? node->right : node->left; \
            if (nullptr == node->left) { \
                stream_buffer.push_back(node->ch); \
                node = root_; \
            }

        char ch = *pencode;
        while (encode_8_bit_size) {
            LOOP_INNER_(0);
            LOOP_INNER_(1);
            LOOP_INNER_(2);
            LOOP_INNER_(3);
            LOOP_INNER_(4);
            LOOP_INNER_(5);
            LOOP_INNER_(6);
            LOOP_INNER_(7);
            ch = *(++pencode);
            encode_8_bit_size -= 8;
        }

        #undef LOOP_INNER_
    }
    // remain bits
    {
        int bit_off = 0;
        char ch = *pencode;
        while (encode_remain_bit_size) {
            node = (ch & (1<<bit_off)) ? node->right : node->left;
            bit_off++;
            if (!(bit_off = (bit_off & 7))) ch = *(++pencode);
            encode_remain_bit_size--;
            if (nullptr == node->left) {
                stream_buffer.push_back(node->ch);
                node = root_;
            }
        }
    }
    if (node != root_) {
        LOG_ERR << "missing some bits.";
        return false;
    }
    return true;
}

}  // namespace huffman

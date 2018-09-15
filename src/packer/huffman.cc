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
#include <iostream>
#include "huffman.h"

namespace huffman {

/** @brief encode a stream buffer use huffman
 *  @param stream_buffer input stream buffer
 *  @param encode_buffer outout encode buffer, code table | int(encode_bit_size) | encode buffer
 *  @return success or fail
 */
bool Huffman::Encode(const std::vector<char> &stream_buffer, std::vector<char> &encode_buffer) {
    encode_buffer.clear();

    if (!BuildFreqMap(stream_buffer)) {
        LOG_ERR << "build frequency map error.";
        return false;
    }

    if (!BuildTree()) {
        LOG_ERR << "build huffman tree error.";
        return false;
    }

    if (!BuildCodeTable()) {
        LOG_ERR << "build huffman code table error.";
        return false;
    }

    huffman_code_t result;
    for (auto ch : stream_buffer) {
        const huffman_code_t &code = code_table_[ch];
        result.insert(result.end(), code.begin(), code.end());
    }

    // write freq map
    if (!WriteFreqMap(encode_buffer)) {
        LOG_ERR << "write huffman code table error.";
        return false;
    }

    // append result to encode buffer
    size_t code_table_size = encode_buffer.size();
    size_t result_size = (result.size() + 7)/8;
    // the new elements are initialized as copies of 0
    encode_buffer.resize(code_table_size + result_size + sizeof(uint64_t), 0);

    char *pencode = &encode_buffer[code_table_size];
    *(uint64_t*)pencode = (uint64_t)result.size(); pencode += sizeof(uint64_t);
    for (int i=0; i<result.size(); i++) {
        if (!result[i]) continue;
        char &ch = pencode[i/8];
        ch = ch|(1<<(i&7));
    }
    return true;
}

/** @brief build frequency map
 *  @param stream_buffer input stream buffer
 *  @return success or fail
 */
bool Huffman::BuildFreqMap(const std::vector<char> &stream_buffer) {
    char_freq_map_.clear();
    // Make frequency table from a input buffer.
    for (char ch : stream_buffer) {
        char_freq_map_[ch]++;
    }
    return true;
}

/** @brief write frequency map to compressed buffer
 *  @param encode_buffer output encode buffer
 *  @return success or fail
 */
bool Huffman::WriteFreqMap(std::vector<char> &encode_buffer) {
    const int need_size = sizeof(int) + (sizeof(char)+sizeof(int))*char_freq_map_.size();
    encode_buffer.resize(need_size);

    char *pencode = &encode_buffer[0], *ptr_bak = pencode;
    *(int*)pencode = (int)char_freq_map_.size(); pencode += sizeof(int);
    for (auto it : char_freq_map_) {
        *pencode = it.first;
        *(int*)(pencode+sizeof(char)) = it.second;
        pencode += sizeof(char) + sizeof(int);
    }
    if (pencode - ptr_bak != need_size) {
        LOG_ERR << "check write size error.";
        return false;
    }
    return true;
}

/** @brief read frequency map from compressed buffer
 *  @param encode_buffer input encode buffer
 *  @param table_size freq table size
 *  @return success or fail
 */
bool Huffman::ReadFreqMap(const std::vector<char> &encode_buffer, size_t &table_size) {
    char_freq_map_.clear();
    table_size = 0;

    size_t size = encode_buffer.size();
    if (size < sizeof(int)){
        LOG_ERR << "encode size error. size:" << size;
    }
    const char *pencode = &encode_buffer[0];
    const int char_count = *(int*)pencode; pencode += sizeof(int);
    table_size = sizeof(int) + (sizeof(char) + sizeof(int))*char_count;
    if (size < table_size) {
        LOG_ERR << "encode size error. char count:" << char_count << ", size:" << size;
        return false;
    }

    for (int i=0; i<char_count; i++) {
        char_freq_map_[*pencode] = *(int*)(pencode + sizeof(char));
        pencode += sizeof(char) + sizeof(int);
    }
    return true;
}

/** @brief build a Huffman Tree
 *  @return success or fail
 */
bool Huffman::BuildTree() {
    root_ = nullptr;

    // append to priority_queue
    std::priority_queue<HuffmanTree *, std::vector<HuffmanTree *>, CompareTree> char_freq_heap;
    for (auto it : char_freq_map_) {
        char_freq_heap.push(new HuffmanTree(it.first, it.second));
    }

    if (char_freq_heap.size() == 0) return true;
    if (char_freq_heap.size() == 1){
        HuffmanTree *left = char_freq_heap.top();
        HuffmanTree *right = new HuffmanTree('\0', 0); // placeholder node
        root_ = new HuffmanTree(left->ch, left->freq + right->freq, left, right);
        return true;
    }

    // HuffmanTree algorithm: Merge two lowest weight leaf nodes until
    while (char_freq_heap.size() > 1) {
        HuffmanTree *left = char_freq_heap.top();
        char_freq_heap.pop();
        HuffmanTree *right = char_freq_heap.top();
        char_freq_heap.pop();
        root_ = new HuffmanTree(left->ch, left->freq + right->freq, left, right);
        // add to the front positon of the item that priority < root
        char_freq_heap.push(root_);
    }
    return true;
}

/** @brief build code table
 *  @param htree huffman tree
 */
bool Huffman::BuildCodeTable() {
    code_table_.clear();
    // maybe empty stream buffer
    if (nullptr == root_) return true;

    std::deque<std::pair<HuffmanTree *, huffman_code_t> > q;
    q.push_back(std::make_pair(root_, huffman_code_t()));

    while (!q.empty()) {
        HuffmanTree *node = q.front().first;
        huffman_code_t code = q.front().second;
        q.pop_front();
        HuffmanTree *left = node->left;
        HuffmanTree *right = node->right;
        if (nullptr != left && nullptr != right) {
            // HuffmanTree's node is always full
            // left child is appended a 0 and right child a 1.
            huffman_code_t copy_code(code);
            q.push_back(std::make_pair(left, (code.push_back(0), code)));
            q.push_back(std::make_pair(right, (copy_code.push_back(1), copy_code)));
        } else {
            // Leaf node: contains the character
            code_table_.insert(std::make_pair(node->ch, code));
        }
    }
    return true;
}

}  // namespace huffman

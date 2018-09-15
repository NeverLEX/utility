/*
 *  Usage:
 *  1. create a package file
 *      Open(filename, MODE_WRITE);
 *      SetVersion(version);
 *      AddFile(filename);
 *      AddDir(dirname);
 *      ...
 *      Close()
 *  2. test a package file
 *      Open(filename, MODE_READ);
 *      GetVersion();
 *      Extract();
 *      Close();
 *  3. use a package file
 *      Open(filename, MODE_READ);
 *      GetFileSream(filename, file_stream)
 *      ...
 *      Close();
 */

#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include "log.h"
#include "huffman.h"

namespace packer {

static const uint64_t global_stream_head        = 0xf9e8d7c6b5a44a5b;  // 64bit stream head
static const uint64_t global_stream_tail        = 0xb5a44a5b6c7d8e9f;  // 64bit stream tail
static const uint64_t global_index_stream_head  = 0x9f8e7d6c5b4aa4b5;  // 64bit index stream head
static const uint64_t global_index_stream_tail  = 0x5b4aa4b5c6d7e8f9;  // 64bit index stream tail
static const uint64_t global_zero_alignment     = 0x0000000000000000;  // 64bit zero alignment

enum OpenMode {
    MODE_UNKNOWN = 0,
    MODE_WRITE   = 1,
    MODE_READ    = 2
};

class Packer {
public:
    Packer() { Reset(); }
    ~Packer() { Close(); }

    /** @brief add a single file to Package
     *  @param filename file name
     *  @param dstpath destination path, root path by default
     */
    bool AddFile(const char *filename, const char *dstpath=nullptr);

    /** @brief check if filename exist
     *  @param filename file name
     */
    bool FileExist(const char *filename);

    /** @brief add a directory to Package
     *  @param path source path
     *  @param dstpath destination path, root path by default
     */
    bool AddDir(const char *path, const char *dstpath=nullptr);

    /** @brief get file stream
     *  @param filename filename in Package
     *  @param file_stream outout file stream
     */
    template<typename streambuf_t>
    bool GetFileStream(const char *filename, streambuf_t &file_stream);

    /** @brief open a package file
     *  @param filename package file name
     *  @param mode open mode
     */
    bool Open(const char *filename, OpenMode mode);

    /** @brief close package file name
     *  @param null
     */
    bool Close();

    /** @brief set resource version
     *  @param version resource version (format: xx.xx[.xx][.xx], x must be digit, eg.  3.14 / 3.14.1 / 3.14.15.92)
     */
    void SetVersion(const char *version);

    /** @brief get resource version
     */
    std::string GetVersion();

    /** @brief extract a package file
     *  @param dstpath extract path, current path by default
     */
    bool Extract(const char *dstpath=nullptr);

    /** @brief extract a single file to a tempfile
     *  @param filename filename in Package
     *  @param prefix prefix of temp file name
     *  @return tempfile name, return empty string if create tempfile failed.
     */
    std::string ExtractToTempFile(const char *filename, const char *prefix);

    /** @brief delete a tempfile created by Packer
     *  @param tempfile temp file name
     */
    static void DeleteTempFile(const std::string &tempfile);

private:
    /** @brief reset
     *  @return null
     */
    void Reset() {
        cur_offset_ = 0;
        version_ = 0;
        open_mode_ = MODE_UNKNOWN;
        file_name_.clear();
        if (if_stream_.is_open()) if_stream_.close();
        if (of_stream_.is_open()) of_stream_.close();
        file_index_.clear();
    }

    /** @brief extract single file
     *  @param dstpath extract path
     *  @param filename extract filename
     *  @param file_stream file stream
     */
    bool ExtractFile(const char *dstpath, const char *filename, const std::vector<char> &file_stream);

    /** @brief add file stream to Package file
     *  @param filename file name
     *  @param dstpath destination path
     */
    bool AddStream(const std::vector<char> &file_stream, const char *filename, const char *dstpath);

    /** @brief joint path
     *  @param path path name
     *  @param name file name
     *  @param newpath new path name
     */
    void JointPath(const char *path, const char *name, char *newpath);

    /** @brief mkdirs
     *  @param fullpath full path
     */
    bool MakeDirs(const char *fullpath);

    // stream file info
    struct StreamInfo {
        uint64_t offset;  // stream offset
        uint64_t size;  // stream size
        StreamInfo(uint64_t off, uint64_t s) : offset(off), size(s) {}
    };

private:
    uint64_t cur_offset_;  // current file offset
    int32_t version_; // file version
    OpenMode open_mode_;  // file open mode
    std::string file_name_;  // package file name
    std::ifstream if_stream_;  // package file stream, READ mode
    std::ofstream of_stream_;  // package file stream, WRITE mode
    std::map<std::string, StreamInfo> file_index_;  // file index in package file
};

/** @brief get file stream
 *  @param filename filename in Package
 *  @param file_stream outout file stream
 */
template<typename streambuf_t>
bool Packer::GetFileStream(const char *filename, streambuf_t &file_stream) {
    if (open_mode_ != MODE_READ) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_READ.";
        return false;
    }

    file_stream.clear();
    std::map<std::string, StreamInfo>::const_iterator it = file_index_.find(filename);
    if (it == file_index_.end()) {
        LOG_ERR << "can not find filename:" << filename;
        return false;
    }
    const StreamInfo & si = it->second;
    uint64_t stream_head = 0, stream_tail = 0;
    if (si.size < sizeof(stream_head) + sizeof(stream_tail)) {
        LOG_ERR << "stream size error, size:" << si.size;
        return false;
    }

    std::vector<char> encode_stream;
    uint64_t size = si.size - sizeof(stream_head) - sizeof(stream_tail);
    encode_stream.resize(size);
    if_stream_.seekg(si.offset, std::ios::beg);
    if_stream_.read((char*)&stream_head, sizeof(stream_head));
    if_stream_.read(&encode_stream[0], size);
    if_stream_.read((char*)&stream_tail, sizeof(stream_tail));
    if (!if_stream_.good()) {
        LOG_ERR << "read file error.";
        return false;
    }

    if (global_stream_head != stream_head || global_stream_tail != stream_tail) {
        LOG_ERR << "check stream head or tail error.";
        return false;
    }

    huffman::Huffman huffman_decode;
    if (!huffman_decode.Decode(encode_stream, file_stream)) {
        LOG_ERR << "decode error.";
        return false;
    }
    return true;
}

// ifmstrem   input file / memory stream
class ifmstream : public std::istream {
    enum StreamMode{
        MODE_UNKNOW = 0,
        MODE_FILE = 1,
        MODE_MEM = 2
    };

    static std::stringbuf & __InnerStringBuf() { static std::stringbuf _(std::ios::in); return _; }

public:
    ifmstream() : std::istream(&__InnerStringBuf()), is_open_(false), stream_mode_(MODE_UNKNOW), string_buf_(std::ios::in) {}
    ~ifmstream() { reset(); }

    bool is_open() const { return is_open_; }

    ifmstream& set_membuf(std::string &&mem_stream) {
        reset();
        stream_mode_ = MODE_MEM;
        string_buf_.str(mem_stream);
        rdbuf(&string_buf_);
        is_open_ = true;
        return *this;
    }

    ifmstream& set_membuf(std::string &mem_stream) {
        reset();
        stream_mode_ = MODE_MEM;
        string_buf_.str(mem_stream);
        rdbuf(&string_buf_);
        is_open_ = true;
        return *this;
    }

    ifmstream& set_filebuf(const char* filename) {
        reset();
        stream_mode_ = MODE_FILE;
        file_buf_.open(filename, std::ios::in);
        is_open_ = file_buf_.is_open();
        if (is_open_) rdbuf(&file_buf_);
        return *this;
    }

    ifmstream& set_filebuf(const char* filename, std::ios::openmode mode) {
        reset();
        stream_mode_ = MODE_FILE;
        file_buf_.open(filename, std::ios::in | mode);
        is_open_ = file_buf_.is_open();
        if (is_open_) rdbuf(&file_buf_);
        return *this;
    }

    void reset() {
        if (stream_mode_ == MODE_FILE) {
            file_buf_.close();
        } else if (stream_mode_ == MODE_MEM) {
            string_buf_.str("");
        }
        is_open_ = false;
        this->clear();
    }

private:
    bool is_open_;
    StreamMode stream_mode_;
    std::stringbuf string_buf_;
    std::filebuf file_buf_;
};

}  // namespace packer

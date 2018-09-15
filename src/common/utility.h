#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

namespace utility {

class Timer {

public:
    Timer() { reset(); }

    void reset() __attribute__((noinline)) {
        elapsed_usec_ = 0;
        memset(&start_, 0, sizeof(start_));
        memset(&stop_, 0, sizeof(stop_));
        start();
    }

    void start() __attribute__((noinline)) { gettimeofday(&start_, 0); }

    void stop() __attribute__((noinline)) {
        if (start_.tv_sec != 0 || start_.tv_usec != 0) {
            gettimeofday(&stop_, 0);
            elapsed_usec_ += (stop_.tv_sec - start_.tv_sec) * 1e6;
            elapsed_usec_ += stop_.tv_usec - start_.tv_usec;
        }
        memset(&start_, 0, sizeof(start_));
        memset(&stop_, 0, sizeof(stop_));
    }

    void pause() __attribute__((noinline)) { stop(); }

    double elapsed_ms() __attribute__((noinline)) {
        stop();
        start();
        return double(elapsed_usec_)/1000;
    }

    double total_ms() __attribute__((noinline)) { return elapsed_ms(); }

private:
    int64_t elapsed_usec_;
    struct timeval start_, stop_;
};

static uint8_t * mem_alloc(size_t size, const char * name) {
    if(void * p=malloc(size)){
        return (uint8_t *)p;
    }
    std::cout << "mem_alloc `" << name << "' error." << std::endl;
    return NULL;
}

static uint8_t * mem_realloc(void * ptr, size_t size, const char * name) {
    if(void * p=realloc(ptr, size)){
        return (uint8_t *)p;
    }
    std::cout << "mem_realloc `" << name << "' error." << std::endl;
    return NULL;
}

static inline void mem_free(void *& p) {
    if(p) free(p), p=NULL;
}

template <const unsigned enlarge_x_8 = 12>
struct buffer_t {
    void * buf_;
    size_t cap_;
    const char * name_;

    buffer_t(const char * name=NULL) {
        name_ = name;
        buf_ = NULL;
        cap_ = 0;
    }
    ~buffer_t() {
        cap_=0, mem_free(buf_), buf_=NULL;
    }
    void rename(const char * name) {
        name_ = name;
    }
    void * reserve(size_t nBytes) {
        if(cap_ < nBytes) {
            size_t newcap_ = size_t(nBytes*double(enlarge_x_8)/8);
            void * newbuf_ = mem_realloc(buf_, newcap_, name_);
            if(!newbuf_) {
                mem_free(buf_);
                newbuf_ = mem_alloc(newcap_, name_);
                if(!newbuf_) {
                    throw "memory error";
                }
            }
            cap_ = newcap_;
            buf_ = newbuf_;
        }
        return buf_;
    }
};

template <typename elem_t, const unsigned enlarge_x_8 = 12> // (12/8) = 1.5
struct array_t: buffer_t<enlarge_x_8> {
    array_t(const char * name=NULL): buffer_t<enlarge_x_8>(name)    { ; }

    inline elem_t & operator [] (size_t i)                { return ((elem_t*)buffer_t<enlarge_x_8>::buf_)[i]; }
    inline const elem_t & operator [] (size_t i) const    { return ((const elem_t*)buffer_t<enlarge_x_8>::buf_)[i]; }

    elem_t * reserve(size_t nElem)                    { return (elem_t *)buffer_t<enlarge_x_8>::reserve(nElem*sizeof(elem_t)); }
    elem_t * resize(size_t nElem)                    { return (elem_t *)buffer_t<enlarge_x_8>::reserve(nElem*sizeof(elem_t)); }

    elem_t * resize(size_t nElem, elem_t val) {
        elem_t * const p = resize(nElem);
        while(nElem) p[--nElem] = val;
        return p;
    }
};

/** @brief mkdirs
 *  @param fullpath full path
 */
static bool MakeDirs(const char *fullpath) {
    char path[512], *r = path;
    strcpy(path, fullpath);
    while(*r) {
        if (*r == '/') {
            *r = '\0';
            if (*path && access(path, 0) != 0) {
                if (mkdir(path, 0755) == -1) return false;
            }
            *r = '/';
        }
        r++;
    }
    if (*path && access(path, 0) != 0) {
        if (mkdir(path, 0755) == -1) return false;
    }
    return true;
}

}  // namespace utility

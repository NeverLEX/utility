#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <map>
#include <vector>
#include <cstring>
#include "packer.h"

namespace packer {

/** @brief add a single file to Package
 *  @param filename file name
 *  @param dstpath destination path
 */
bool Packer::AddFile(const char *filename, const char *dstpath) {
    if (open_mode_ != MODE_WRITE) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_WRITE.";
        return false;
    }

    if (nullptr==filename || *filename=='\0') return false;
    if (file_index_.find(filename)!=file_index_.end()) {
        LOG_ERR << "conflict name in package file";
        return false;
    }

    std::ifstream new_file(filename, std::ios::binary);
    if (!new_file.is_open()) {
        LOG_ERR << "open file error. filename: " << filename;
        return false;
    }
    // get file size
    new_file.seekg(0, std::ios::end);
    uint64_t size = new_file.tellg();
    new_file.seekg(0, std::ios::beg);
    // read file
    std::vector<char> file_stream(size);
    // check if good
    if (!new_file.read(&file_stream[0], size)) {
        LOG_ERR << "read file error. filename: " << filename;
        new_file.close();
        return false;
    }
    // check size is ok.
    if (new_file.gcount() != size) {
        LOG_ERR << "check size error.";
        new_file.close();
        return false;
    }
    // set as current in Packer
    if (nullptr == dstpath) dstpath = "";
    // get filename
    const char *s_name = filename + strlen(filename);
    while(s_name > filename && *s_name!='/') s_name--;
    if (*s_name=='/') s_name++;
    // add stream to Packer
    AddStream(file_stream, s_name, dstpath);
    new_file.close();
    return true;
}

/** @brief check if filename exist
 *  @param filename file name
 */
bool Packer::FileExist(const char *filename) {
    if (nullptr==filename || *filename=='\0') return false;
    return file_index_.find(filename)!=file_index_.end();
}

/** @brief add a directory to Package
 *  @param path source path
 *  @param dstpath destination path
 */
bool Packer::AddDir(const char *path, const char *dstpath) {
    if (open_mode_ != MODE_WRITE) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_WRITE.";
        return false;
    }

    if (nullptr == path) {
        LOG_ERR << "path is null";
        return false;
    }
    // check if path is a valid dir
    struct stat s;
    lstat(path, &s);
    if (!S_ISDIR(s.st_mode)) {
        LOG_ERR << "path is not a valid directory!";
        return false;
    }

    DIR *dir = opendir(path);
    if (nullptr == dir) {
        LOG_ERR << "can not open dir" << path;
        return false;
    }

    // recursively add files
    struct dirent * filename;
    char sub_path[512], sub_dstpath[512];
    if (nullptr == dstpath) dstpath = "";
    while ((filename = readdir(dir)) != nullptr) {
        const char *name = filename->d_name;
        if ((name[0]=='.' && name[1]=='\0') || (name[0]=='.' && name[1]=='.' && name[2]=='\0')) continue;
        JointPath(path, name, sub_path);
        JointPath(dstpath, name, sub_dstpath);
        lstat(sub_path, &s);
        if (S_ISDIR(s.st_mode)) {
            // name is a dir, add dir
            if (!AddDir(sub_path, sub_dstpath)) return false;
        } else {
            // name is a file, add file
            if (!AddFile(sub_path, dstpath)) return false;
        }
    }
    return true;
}

/** @brief open a package file
 *  @param filename package file name
 *  @param mode open mode
 */
bool Packer::Open(const char *filename, OpenMode mode) {
    if (nullptr==filename) {
        LOG_ERR << "filename is null";
        return false;
    }

    if (open_mode_ != MODE_UNKNOWN) this->Close();
    this->Reset();

    file_name_ = filename;
    open_mode_ = mode;
    if (MODE_WRITE == open_mode_) {
        of_stream_.open(filename, std::ios::binary);
        of_stream_.write((char*)&cur_offset_, sizeof(cur_offset_));
        cur_offset_ += sizeof(cur_offset_);
        return of_stream_.is_open();
    } else if (MODE_READ == open_mode_) {
        if_stream_.open(filename, std::ios::binary);
        if (!if_stream_.is_open()) {
            LOG_ERR << "open file error. filename:" << filename;
            return false;
        }
        uint64_t index_offset = 0, file_size = 0;
        if_stream_.seekg(0, std::ios::end);
        file_size = if_stream_.tellg();
        if_stream_.seekg(0, std::ios::beg);
        if (file_size < sizeof(index_offset)) {
            LOG_ERR << "file size error.";
            return false;
        }
        if_stream_.read((char*)&index_offset, sizeof(index_offset));
        if (index_offset & 7) {
            LOG_ERR << "check index offset 64bit alignment error.";
            return false;
        }
        int index_size = 0;
        if (file_size < index_offset + sizeof(global_index_stream_head) + sizeof(version_) + sizeof(index_size) + sizeof(global_index_stream_tail)) {
            LOG_ERR << "file size error.";
            return false;
        }
        // read index stream
        if_stream_.seekg(index_offset, std::ios::beg);
        std::vector<char> index_stream(file_size - index_offset);
        if_stream_.read(&index_stream[0], file_size - index_offset);
        if (!if_stream_.good()) {
            LOG_ERR << "read index stream error.";
            return false;
        }
        if (index_stream.size() & 7) {
            LOG_ERR << "check 64bit alignment error.";
            return false;
        }
        // check head and tail
        const char *index = &index_stream[0], *index_end = index + index_stream.size() - sizeof(global_index_stream_tail);
        if (*(uint64_t*)index != global_index_stream_head) {
            LOG_ERR << "check index stream head error.";
            return false;
        }
        if (*(uint64_t*)index_end != global_index_stream_tail) {
            LOG_ERR << "check index stream tail error.";
            return false;
        }
        index += sizeof(global_index_stream_head);
        index_size = *(int32_t*)index; index += sizeof(index_size);
        version_ = *(int32_t*)index; index += sizeof(version_);
        index_end = index + index_size - sizeof(index_size) - sizeof(version_);
        // parse index stream, and create index map
        while(index < index_end) {
            if (index+sizeof(uint32_t) > index_end) return false;
            // filename len
            uint32_t len = *(uint32_t*)index;
            index += sizeof(uint32_t);
            if (index+len+sizeof(StreamInfo) > index_end) return false;
            // filename & StreamInfo
            std::string name(index, len);
            StreamInfo stream_info = *(StreamInfo*)(index+len);
            file_index_.insert(std::make_pair(name, stream_info));
            index += len+sizeof(StreamInfo);
        }
        return index==index_end;
    }
    LOG_ERR << "unknow mode: " << mode;
    return false;
}

/** @brief close package file name
 *  @param null
 */
bool Packer::Close() {
    if (MODE_READ == open_mode_) {
        if_stream_.close();
        Reset();
        return true;
    }
    if (MODE_WRITE != open_mode_) {
        //LOG_ERR << "unknow mode: " << open_mode_;
        //return false;
        Reset();
        return true;
    }
    // write file
    of_stream_.seekp(0, std::ios::beg);
    of_stream_.write((char*)&cur_offset_, sizeof(cur_offset_));
    of_stream_.seekp(0, std::ios::end);
    of_stream_.write((char*)&global_index_stream_head, sizeof(global_index_stream_head));
    int index_size = 0;
    std::ios::pos_type pos = of_stream_.tellp();
    of_stream_.write((char*)&index_size, sizeof(version_));
    of_stream_.write((char*)&version_, sizeof(version_));
    index_size = sizeof(index_size) + sizeof(version_);
    // write file index
    for (std::map<std::string, StreamInfo>::const_iterator it = file_index_.begin(); it!=file_index_.end(); it++) {
        uint32_t len = it->first.length();
        of_stream_.write((char*)&len, sizeof(len));
        of_stream_.write(it->first.c_str(), len);
        of_stream_.write((char*)&it->second, sizeof(it->second));
        index_size += sizeof(len) + len + sizeof(it->second);
    }
    // 64bit alignment
    if (const int align_size = 7&-(int)index_size) of_stream_.write((char*)&global_zero_alignment, align_size);
    of_stream_.write((char*)&global_index_stream_tail, sizeof(global_index_stream_tail));
    // write index size
    of_stream_.seekp(pos, std::ios::beg);
    of_stream_.write((char*)&index_size, sizeof(version_));
    bool ok = of_stream_.good();
    of_stream_.close();
    Reset();
    return ok;
}

/** @brief set resource version
 *  @param version resource version (format: xx.xx[.xx][.xx], x must be digit, eg.  3.14 / 3.14.1 / 3.14.15.92)
 */
void Packer::SetVersion(const char *version) {
    int version_val = 0, val = 0;
    const char *r = version, *e = r + std::strlen(version);
    while(r < e) {
        if (!('0'<=*r && *r<='9')) {
            LOG_WARN << "format must be: xx.xx[.xx][.xx], x is a digit, eg. 3.14 / 3.14.1 / 3.14.15.92";
            return;
        }
        val = 0;
        while(r<e && '0'<=*r && *r<='9') val = val*10 + (*r++ - '0');
        if (version_val & 0xFF000000){
            LOG_WARN << "format must be: xx.xx[.xx][.xx], x is a digit, eg. 3.14 / 3.14.1 / 3.14.15.92";
            return;
        }
        version_val = (version_val<<8) + (val|0x80);
        if (val >= 100) {
            LOG_WARN << "format must be: xx.xx[.xx][.xx], x is a digit, eg. 3.14 / 3.14.1 / 3.14.15.92";
            return;
        }
        if (r==e) break;
        if (*r=='.' && r[1]) { r++; continue; }
        LOG_WARN << "format must be: xx.xx[.xx][.xx], x is a digit, eg. 3.14 / 3.14.1 / 3.14.15.92";
        return;
    }
    version_ = version_val;
}

/** @brief get resource version
*/
std::string Packer::GetVersion() {
    if (version_ == 0) return "";
    char buffer[64], *w = buffer;
    const char *r = (const char *)&version_;
    for (int i=sizeof(version_)-1; i>=0; i--) {
        if (!(r[i] & 0x80)) continue;
        w += sprintf(w, i==0 ? "%d" : "%d.", r[i] & 0x7F);
    }
    *w = '\0';
    return buffer;
}

/** @brief extract a package file
 *  @param dstpath extract path, current path by default
 */
bool Packer::Extract(const char *dstpath) {
    if (open_mode_ != MODE_READ) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_READ.";
        return false;
    }

    const char *path = nullptr==dstpath ? "" : dstpath;
    std::vector<char> file_stream;
    for (std::map<std::string, StreamInfo>::const_iterator it = file_index_.begin(); it != file_index_.end(); it++) {
        if (!GetFileStream(it->first.c_str(), file_stream)) {
            LOG_ERR << "extract file error, filename:" << it->first;
            return false;
        }
        if (!ExtractFile(path, it->first.c_str(), file_stream)) {
            LOG_ERR << "create file error, filename:" << it->first;
            return false;
        }
    }
    return true;
}

/** @brief extract single file
 *  @param dstpath extract path
 *  @param filename extract filename
 *  @param file_stream file stream
 */
bool Packer::ExtractFile(const char *dstpath, const char *filename, const std::vector<char> &file_stream) {
    if (open_mode_ != MODE_READ) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_READ.";
        return false;
    }

    char fullpath[512];
    JointPath(dstpath, filename, fullpath);
    if (!MakeDirs(fullpath)) {
        LOG_ERR << "mkdir error, fullpath:" << fullpath;
        return false;
    }
    std::ofstream ofh(fullpath, std::ios::binary);
    if (!ofh.is_open()) {
        LOG_ERR << "open file error, filename:" << fullpath;
        return false;
    }
    ofh.write((char*)&file_stream[0], file_stream.size());
    if (!ofh.good()) {
        LOG_ERR << "write file error, filename:" << fullpath;
        return false;
    }
    ofh.close();
    return true;
}

/** @brief add file stream to Package
 *  @param filename file name
 *  @param dstpath destination path
 */
bool Packer::AddStream(const std::vector<char> &file_stream, const char *filename, const char *dstpath) {
    if (open_mode_ != MODE_WRITE) {
        LOG_ERR << "check open mode fail. OpenMode is not MODE_WRITE.";
        return false;
    }

    if (nullptr==filename || nullptr==dstpath) {
        LOG_ERR << "filename/dstpath error.";
        return false;
    }
    // encode
    huffman::Huffman huffman_encode;
    std::vector<char> encode_stream;
    if (!huffman_encode.Encode(file_stream, encode_stream)) {
        LOG_ERR << "encode error.";
        return false;
    }
    // write file stream
    of_stream_.write((char*)&global_stream_head, sizeof(global_stream_head));
    of_stream_.write(&encode_stream[0], encode_stream.size()*sizeof(char));
    // 64bit alignment
    const int align_size = 7&-(int)encode_stream.size();
    if (align_size) of_stream_.write((char*)&global_zero_alignment, align_size);
    of_stream_.write((char*)&global_stream_tail, sizeof(global_stream_tail));
    // set index
    char inner_name[512];
    JointPath(dstpath, filename, inner_name);
    uint64_t stream_size = sizeof(global_stream_head) + sizeof(global_stream_tail) + encode_stream.size()*sizeof(char) + align_size;
    file_index_.insert(std::make_pair(inner_name, StreamInfo(cur_offset_, stream_size)));
    // mode to next file
    cur_offset_ += stream_size;
    return of_stream_.good();
}

/** @brief joint path
 *  @param path path name
 *  @param name file name
 *  @param newpath new path name
 */
void Packer::JointPath(const char *path, const char *name, char *newpath) {
    if (nullptr==path || nullptr==name || nullptr==newpath) return;
    const int len = strlen(path);
    if (0==len) {
        strcpy(newpath, name);
    } else {
        bool has_slash = (path[len-1]=='/');
        sprintf(newpath, has_slash ? "%s%s" : "%s/%s", path, name);
    }
}

/** @brief mkdirs
 *  @param fullpath full path
 */
bool Packer::MakeDirs(const char *fullpath) {
    char path[512], *r = path;
    std::strcpy(path, fullpath);
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
    return true;
}

/** @brief extract a single file to a tempfile
 *  @param filename filename in Package
 *  @param prefix prefix of temp file name
 *  @return tempfile name, return empty string if create tempfile failed.
 */
std::string Packer::ExtractToTempFile(const char *filename, const char *prefix) {
    std::vector<char> mem_stream;
    if (!GetFileStream(filename, mem_stream)) return "";

    char tempfile[512];
    sprintf(tempfile, "%sXXXXXXXX", prefix);
    // mkdtemp will replace XXXXXXXX with a unique alphanumeric combination, see `man mkdtemp 3` for detail
    int fd = mkstemp(tempfile);
    if (fd==-1) return "";
    ::write(fd, &mem_stream[0], mem_stream.size());
    ::close(fd);
    return tempfile;
}

/** @brief delete a tempfile created by Packer
 *  @param tempfile temp file name
 */
void Packer::DeleteTempFile(const std::string &tempfile) {
    if (!tempfile.empty()) unlink(tempfile.c_str());
}

}  // namespace packer

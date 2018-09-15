#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "unistd.h"
#include "log.h"

#define __USE_CUSTOM_TEST__
#ifdef __USE_CUSTOM_TEST__
#include "ctest.h"
#else
#include "gtest/gtest.h"
#endif

#define private public  // hack complier
#define protected public
#include "packer.h"
#undef private
#undef protected

/*
 * set global environment
 */
class PackerEnvironment : public testing::Environment {
public:
    bool ParseOption(int argc, char **argv) {
        for (int i=1; i<argc; i++) {
            if (0==strncmp(argv[i], "--data_path=", 12)){
                test_data_path = argv[i] + 12;
                return true;
            }
        }
        std::cout << "Usage: Packer_test --data_path=datapath" << std::endl;
        return false;
    }


protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

private:
public:
    std::string test_data_path;
};

PackerEnvironment *env;

namespace packer{
namespace {

/*
 * PackerTest, use googletest
 */
class PackerTest: public Packer, public ::testing::Test {

public:
    static void SetUpTestCase() {
    }

    static void TearDownTestCase() {

    }

protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {

    }

protected:

    bool IsSameFile(const char *srcpath, const char *dstpath) {
        std::ifstream srcfh(srcpath, std::ios::binary), dstfh(dstpath, std::ios::binary);
        if (!srcfh.is_open() || !dstfh.is_open()) return false;
        const char *res = [] (std::ifstream &srcfh, std::ifstream &dstfh) -> const char * {
            // get file size
            srcfh.seekg(0, std::ios::end);
            uint64_t src_size = srcfh.tellg();
            srcfh.seekg(0, std::ios::beg);

            dstfh.seekg(0, std::ios::end);
            uint64_t dst_size = dstfh.tellg();
            dstfh.seekg(0, std::ios::beg);
            if (src_size != dst_size) return "check size error";

            // read file
            std::vector<char> src_file_stream(src_size);
            std::vector<char> dst_file_stream(dst_size);
            // check if good
            if (!srcfh.read(&src_file_stream[0], src_size)) return "read src file error";
            if (!dstfh.read(&dst_file_stream[0], dst_size)) return "read dst file error";

            // check size is ok.
            if (srcfh.gcount() != src_size) return "check src size error";
            if (dstfh.gcount() != dst_size) return "check src size error";

            // check stream
            if (src_file_stream != dst_file_stream) return "file is not same.";
            return NULL;
        } (srcfh, dstfh);
        srcfh.close();
        dstfh.close();
        return res == NULL;
    }

    bool IsSameDir(const char *srcpath, const char *dstpath) {
        if (nullptr == srcpath || nullptr == dstpath) return false;
        // check if path is a valid dir
        struct stat s;
        lstat(srcpath, &s);
        if (!S_ISDIR(s.st_mode)) return false;

        DIR *dir = opendir(srcpath);
        if (nullptr == dir) return false;

        // recursively add files
        struct dirent * filename;
        char sub_srcpath[512], sub_dstpath[512];
        while ((filename = readdir(dir)) != nullptr) {
            const char *name = filename->d_name;
            if ((name[0]=='.' && name[1]=='\0') || (name[0]=='.' && name[1]=='.' && name[2]=='\0')) continue;
            JointPath(srcpath, name, sub_srcpath);
            JointPath(dstpath, name, sub_dstpath);
            lstat(sub_srcpath, &s);
            if (S_ISDIR(s.st_mode)) {
                if (!IsSameDir(sub_srcpath, sub_dstpath)) return false;
            } else {
                if (!IsSameFile(sub_srcpath, sub_dstpath)) {
                    std::cout << "check file error. file1:" << sub_srcpath << ", file2:" << sub_dstpath << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    void RemoveDir(const char *srcpath) {
         if (nullptr == srcpath) return;
        // check if path is a valid dir
        struct stat s;
        lstat(srcpath, &s);
        if (!S_ISDIR(s.st_mode)) return;

        DIR *dir = opendir(srcpath);
        if (nullptr == dir) return;

        // recursively add files
        struct dirent * filename;
        char sub_srcpath[512];
        while ((filename = readdir(dir)) != nullptr) {
            const char *name = filename->d_name;
            if ((name[0]=='.' && name[1]=='\0') || (name[0]=='.' && name[1]=='.' && name[2]=='\0')) continue;
            JointPath(srcpath, name, sub_srcpath);
            lstat(sub_srcpath, &s);
            if (S_ISDIR(s.st_mode)) {
                RemoveDir(sub_srcpath);
            } else {
                remove(sub_srcpath);
            }
        }
        rmdir(srcpath);
    }

    void AddFileTest(const std::string &testdatapath) {
        std::string path = testdatapath;
        if (path.back() == '/') path.resize(path.length()-1);
        //AddFileTest
        std::string tmp_file = path + "/temp.txt";
        std::ofstream fh(tmp_file, std::ios::binary);
        EXPECT_TRUE(fh.is_open());
        std::string str = "aiu239s35893123\x12\x19\x19\x00\x91\x23\x98\xff";
        fh.write(str.c_str(), str.length());
        fh.close();

        Packer res_packer;
        std::string tmp_path  = path + "_tmpfile";
        EXPECT_TRUE(res_packer.Open("test_file", MODE_WRITE));
        EXPECT_FALSE(res_packer.FileExist("temp.txt"));
        res_packer.AddFile(tmp_file.c_str());
        EXPECT_TRUE(res_packer.FileExist("temp.txt"));
        res_packer.Close();

        EXPECT_TRUE(res_packer.Open("test_file", MODE_READ));
        res_packer.Extract(tmp_path.c_str());
        res_packer.Close();
        std::string extract_file = tmp_path + "/temp.txt";
        EXPECT_TRUE(IsSameFile(tmp_file.c_str(), extract_file.c_str()));
        remove(tmp_file.c_str());
        remove("test_file");
        RemoveDir(tmp_path.c_str());
    }

    void AddDirTest(const std::string &testdatapath) {
        std::string path = testdatapath;
        if (path.back() == '/') path.resize(path.length()-1);

        Packer res_packer;
        std::string tmp_path  = path + "_tmp";
        EXPECT_TRUE(res_packer.Open("test", MODE_WRITE));
        res_packer.AddDir(path.c_str());
        res_packer.Close();

        EXPECT_TRUE(res_packer.Open("test", MODE_READ));
        res_packer.Extract(tmp_path.c_str());
        res_packer.Close();

        EXPECT_TRUE(IsSameDir(path.c_str(), tmp_path.c_str()));
        remove("test");
        RemoveDir(tmp_path.c_str());
    }

    void TempFileTest(const std::string &testdatapath) {
        std::string path = testdatapath;
        if (path.back() == '/') path.resize(path.length()-1);
        //AddFileTest
        std::string tmp_file = path + "/hellotemp.txt";
        std::ofstream fh(tmp_file, std::ios::binary);
        EXPECT_TRUE(fh.is_open());
        std::string str = "afqetqw3rqaiu239s35893123\x12\x19\x19\x00\x91\x23\x98\xff";
        fh.write(str.c_str(), str.length());
        fh.close();

        Packer res_packer;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_WRITE));
        EXPECT_FALSE(res_packer.FileExist("hellotemp.txt"));
        res_packer.AddFile(tmp_file.c_str());
        EXPECT_TRUE(res_packer.FileExist("hellotemp.txt"));
        res_packer.Close();

        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_READ));
        std::string system_temp_file_name = res_packer.ExtractToTempFile("hellotemp.txt", "hhhhaeoafaef_");
        EXPECT_TRUE(system_temp_file_name != "");
        res_packer.Close();
        EXPECT_TRUE(IsSameFile(system_temp_file_name.c_str(), tmp_file.c_str()));
        res_packer.DeleteTempFile(system_temp_file_name);
        std::ifstream ifs(system_temp_file_name.c_str(), std::ios::binary);
        EXPECT_FALSE(ifs.is_open());
        remove(tmp_file.c_str());
        remove("test_tmp_file");
    }

    void BinaryFormatTest() {
        std::vector<char> mem_stream = {'\x01', '\x02', '\x05', '\x00', '\x20', '\x34'};

        Packer res_packer;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_WRITE));
        res_packer.AddStream(mem_stream, "temp_inner_file", "");
        res_packer.Close();

        std::string str_lab;
        std::vector<char> read_stream;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_READ));
        res_packer.GetFileStream("temp_inner_file", read_stream);
        res_packer.GetFileStream("temp_inner_file", str_lab);
        res_packer.Close();

        EXPECT_TRUE(mem_stream == read_stream);
        char buffer[256];
        ifmstream ifms;
        ifms.set_membuf(str_lab);
        EXPECT_TRUE(ifms.read(buffer, 3));
        EXPECT_TRUE(ifms.gcount() == 3);
        EXPECT_TRUE(0==memcmp(buffer, &mem_stream[0], 3));
        EXPECT_FALSE(ifms.read(buffer, 5));
        EXPECT_TRUE(ifms.gcount() == 3);
        EXPECT_TRUE(0==memcmp(buffer, &mem_stream[0]+3, 3));
        EXPECT_FALSE(ifms.read(buffer, 3));
        remove("test_tmp_file");
    }

    void TextFormatTest(){
        std::string test = "hello world\r\n\n\nok\nbad";
        std::vector<char> mem_stream;
        for (int i=0; i<test.size(); i++) mem_stream.push_back(test[i]);

        Packer res_packer;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_WRITE));
        res_packer.AddStream(mem_stream, "temp_inner_file", "");
        res_packer.Close();

        std::string str_stream;
        std::vector<char> read_stream;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_READ));
        res_packer.GetFileStream("temp_inner_file", read_stream);
        res_packer.GetFileStream("temp_inner_file", str_stream);
        res_packer.Close();

        EXPECT_TRUE(mem_stream == read_stream);
        ifmstream ifms;
        ifms.set_membuf(str_stream);

        std::string str;
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="hello world\r");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="ok");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="bad");
        EXPECT_FALSE(std::getline(ifms, str));
        remove("test_tmp_file");
    }

    void VersionTest() {
        Packer res_packer;
        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_WRITE));
        res_packer.SetVersion(".1.1.1");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("1.1.");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("1.a.1");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("1.1.1a");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("1.1.1.1.1");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("112.1.1");
        EXPECT_TRUE(res_packer.GetVersion() == "");
        res_packer.SetVersion("0.0.1");
        EXPECT_TRUE(res_packer.GetVersion() == "0.0.1");
        res_packer.SetVersion("3.14.1");
        EXPECT_TRUE(res_packer.GetVersion() == "3.14.1");
        res_packer.SetVersion("3.14.1.0");
        EXPECT_TRUE(res_packer.GetVersion() == "3.14.1.0");
        res_packer.Close();

        EXPECT_TRUE(res_packer.Open("test_tmp_file", MODE_READ));
        EXPECT_TRUE(res_packer.GetVersion() == "3.14.1.0");
        res_packer.Close();

        remove("test_tmp_file");
    }

};

class IfmstreamTest: public ifmstream, public ::testing::Test {
protected:
    void TestFileMem(const std::string &testdatapath) {
        std::string path = testdatapath;
        if (path.back() == '/') path.resize(path.length()-1);
        //AddFileTest
        std::string tmp_file = path + "/hellotemp.txt";
        std::ofstream fh(tmp_file, std::ios::binary);
        std::string str_file = "afqetqw3rqaiu239s35893123\x12\x19\x19\x00\x91\x23\x98\xff";
        fh.write(str_file.c_str(), str_file.length());
        fh.close();

        std::string mem_stream = "hello world\r\n\nok\nbad";

        ifmstream ifms;
        char buffer[512];
        std::string str;
        int len = str_file.length()/2;
        EXPECT_FALSE(ifms.is_open());
        ifms.set_filebuf(tmp_file.c_str(), std::ios::binary);
        EXPECT_TRUE(ifms.is_open());
        EXPECT_TRUE(ifms.read(buffer, len));
        EXPECT_TRUE(ifms.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str(), len));
        EXPECT_TRUE(ifms.read(buffer, len));
        EXPECT_TRUE(ifms.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str() + len, len));
        EXPECT_FALSE(ifms.read(buffer, 2));

        // set mem
        for (int i=0; i<3; i++) {
            ifms.set_membuf(mem_stream);
            EXPECT_TRUE(ifms.is_open());
            EXPECT_TRUE(std::getline(ifms, str));
            EXPECT_TRUE(str=="hello world\r");
            EXPECT_TRUE(std::getline(ifms, str));
            EXPECT_TRUE(str=="");
            EXPECT_TRUE(std::getline(ifms, str));
            EXPECT_TRUE(str=="ok");
            auto pos = ifms.tellg();
            EXPECT_TRUE(std::getline(ifms, str));
            EXPECT_TRUE(str=="bad");
            ifms.seekg(pos);
            EXPECT_TRUE(std::getline(ifms, str));
            EXPECT_TRUE(str=="bad");
            EXPECT_FALSE(std::getline(ifms, str));
        }

        // set again
        ifms.set_filebuf(tmp_file.c_str(), std::ios::binary);
        EXPECT_TRUE(ifms.is_open());
        EXPECT_TRUE(ifms.read(buffer, len));
        EXPECT_TRUE(ifms.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str(), len));
        EXPECT_TRUE(ifms.read(buffer, len));
        EXPECT_TRUE(ifms.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str() + len, len));
        EXPECT_FALSE(ifms.read(buffer, 2));

        // set mem
        ifms.set_membuf(mem_stream);
        EXPECT_TRUE(ifms.is_open());
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="hello world\r");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="ok");
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str=="bad");
        EXPECT_FALSE(std::getline(ifms, str));

        ifms.set_filebuf(tmp_file.c_str());
        EXPECT_TRUE(ifms.is_open());
        EXPECT_TRUE(std::getline(ifms, str));
        EXPECT_TRUE(str == "afqetqw3rqaiu239s35893123\x12\x19\x19");
        EXPECT_FALSE(std::getline(ifms, str));

        // set again
        std::istream &is = ifms.set_filebuf(tmp_file.c_str(), std::ios::binary);
        EXPECT_TRUE(is.read(buffer, len));
        EXPECT_TRUE(is.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str(), len));
        EXPECT_TRUE(ifms.read(buffer, len));
        EXPECT_TRUE(ifms.gcount() == len);
        EXPECT_TRUE(0==memcmp(buffer, str_file.c_str() + len, len));
        EXPECT_FALSE(is.read(buffer, 2));

        remove(tmp_file.c_str());
    }
};

TEST_F(PackerTest, AddFileTest) { AddFileTest(env->test_data_path); }
TEST_F(PackerTest, AddDirTest) { AddDirTest(env->test_data_path); }
TEST_F(PackerTest, TempFileTest) { TempFileTest(env->test_data_path); }
TEST_F(PackerTest, BinaryFormatTest) { BinaryFormatTest(); }
TEST_F(PackerTest, TextFormatTest) { TextFormatTest(); }
TEST_F(PackerTest, VersionTest) { VersionTest(); }
TEST_F(IfmstreamTest, TestFileMem) { TestFileMem(env->test_data_path); }

}  // namespace
}  // namespace packer

GTEST_API_ int main(int argc, char **argv) {
    env = new PackerEnvironment();
    if (!env->ParseOption(argc, argv)) return 1;
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


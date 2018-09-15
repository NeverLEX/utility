#include <iostream>
#include <fstream>
#include <string.h>
#include "log.h"

#define __USE_CUSTOM_TEST__
#ifdef __USE_CUSTOM_TEST__
#include "ctest.h"
#else
#include "gtest/gtest.h"
#endif

#define private public  // hack complier
#define protected public
#include "huffman.h"
#undef private
#undef protected

/*
 * set global environment
 */
class HuffmanEnvironment : public testing::Environment {
public:
    HuffmanEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

HuffmanEnvironment *env;

namespace huffman {
namespace {

/*
 * HuffmanTest, use googletest
 */
class HuffmanTest: public Huffman, public ::testing::Test {

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

    void EmptyTest() {
        Huffman huffman_encode, huffman_decode;
        std::vector<char> stream_buffer;
        std::vector<char> encode_buffer;
        std::vector<char> stream_buffer_x;
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(stream_buffer == stream_buffer_x);
    }

    void CharTest() {
        Huffman huffman_encode, huffman_decode;
        std::string str = "affaehfanbfizoaaeflkajkedhaejf1273182y761281291240s013ls34ksdguw3";
        std::vector<char> stream_buffer;
        for (auto ch : str) stream_buffer.push_back(ch);
        std::vector<char> encode_buffer;
        std::vector<char> stream_buffer_x;
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(stream_buffer == stream_buffer_x);
    }

    void HexTest() {
        Huffman huffman_encode, huffman_decode;
        std::string str = "affaehfanbfizoaaeflkajkedhaejf1273182y761281291240s013ls34ksdguw3\x12\x43\xff\x00\x78\x12\x43\x00\xff\xe2\x45\x45";
        std::vector<char> stream_buffer;
        for (auto ch : str) stream_buffer.push_back(ch);
        std::vector<char> encode_buffer;
        std::vector<char> stream_buffer_x;
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(stream_buffer == stream_buffer_x);

        stream_buffer.clear();
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(stream_buffer == stream_buffer_x);

        for (int i=0; i<10; i++) stream_buffer.push_back('a');
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(stream_buffer == stream_buffer_x);
    }

    void HexStringTest() {
        Huffman huffman_encode, huffman_decode;
        std::string str = "affaehfanbfizoaaeflkajkedhaejf1273182y761281291240s013ls34ksdguw3\x12\x43\xff\x00\x78\x12\x43\x00\xff\xe2\x45\x45";
        std::vector<char> stream_buffer;
        for (auto ch : str) stream_buffer.push_back(ch);
        std::vector<char> encode_buffer;
        std::string stream_buffer_x;
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE(str == stream_buffer_x);

        stream_buffer.clear();
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE("" == stream_buffer_x);

        for (int i=0; i<10; i++) stream_buffer.push_back('a');
        huffman_encode.Encode(stream_buffer, encode_buffer);
        huffman_decode.Decode(encode_buffer, stream_buffer_x);
        EXPECT_TRUE("aaaaaaaaaa" == stream_buffer_x);
    }


};

TEST_F(HuffmanTest, EmptyTest) { EmptyTest(); }
TEST_F(HuffmanTest, CharTest) { CharTest(); }
TEST_F(HuffmanTest, HexTest) { HexTest(); }
TEST_F(HuffmanTest, HexStringTest) { HexTest(); }

}  // namespace
}  // namespace huffman

GTEST_API_ int main(int argc, char **argv) {
    env = new HuffmanEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/time.h>
#include "log.h"

#define __USE_CUSTOM_TEST__
#ifdef __USE_CUSTOM_TEST__
#include "ctest.h"
#else
#include "gtest/gtest.h"
#endif

#define private public  // hack complier
#define protected public
#include "option-parser.h"
#undef private
#undef protected

/*
 * set global environment
 */
class OptionParserEnvironment : public testing::Environment {
public:
    OptionParserEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

OptionParserEnvironment *env;

namespace optionparser {
namespace {

/*
 * OptionParserTest, use googletest
 */
class OptionParserTest: public ::testing::Test {

protected:

    void NormalTest() {
        OptionParser parser;
        parser.Register("config", "xxxx");
        parser.Register("addr", "xxxx");
        parser.Register("temp", "xxxx", false);
        char *argv[4];
        std::string tmpstr[4] = {
            "binary_name",
            "--config=gm",
            "--addr=127.0.0.1:8080",
            "--temp=xxx"
        };
        for (int i=0; i<4; i++) {
            argv[i] = &tmpstr[i][0];
        }
        EXPECT_TRUE(parser.ParseOptions(4, argv));
        EXPECT_TRUE(parser.GetOption("config") == "gm");
        EXPECT_TRUE(parser.GetOption("addr") == "127.0.0.1:8080");
        EXPECT_TRUE(parser.GetOption("temp") == "xxx");
        EXPECT_TRUE(parser.ParseOptions(3, argv));
        EXPECT_FALSE(parser.ParseOptions(2, argv));
    }
};

TEST_F(OptionParserTest, NormalTest) { NormalTest(); }

}  // namespace
}  // namespace optionparser

GTEST_API_ int main(int argc, char **argv) {
    env = new OptionParserEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


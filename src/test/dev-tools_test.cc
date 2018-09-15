#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "log.h"

#define DEBUG
#define __USE_CUSTOM_TEST__
#ifdef __USE_CUSTOM_TEST__
#include "ctest.h"
#else
#include "gtest/gtest.h"
#endif

#define private public  // hack complier
#define protected public
#include "dev-tools.h"
#undef private
#undef protected

/*
 * set global environment
 */
class DevToolsEnvironment : public testing::Environment {
public:
    DevToolsEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

DevToolsEnvironment *env;

namespace devtools {
namespace {

/*
 * DevToolsTest, use googletest
 */
class DevToolsTest: public ::testing::Test {

protected:

    void ArrayTest() {
        utility::array_t<char> tmp;
        tmp.resize(4, 'a');
        for (int i=0; i<4; i++) {
            EXPECT_TRUE(tmp[i] == 'a');
        }
    }

    void TimerTest() {
        add_time_probe(temp);
        usleep(1e6);
        double x = total_elapsed_ms(temp);
        EXPECT_TRUE(x<=1010 && x>=990);
        pause_clock(temp);
        usleep(1e6);
        start_clock(temp);
        usleep(1e6);
        x = total_elapsed_ms(temp);
        EXPECT_TRUE(x<=2020 && x>=1980);
    }
};

TEST_F(DevToolsTest, ArrayTest) { ArrayTest(); }
TEST_F(DevToolsTest, TimerTest) { TimerTest(); }

}  // namespace
}  // namespace devtools

GTEST_API_ int main(int argc, char **argv) {
    env = new DevToolsEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


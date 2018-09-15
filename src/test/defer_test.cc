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
#include "defer.h"
#undef private
#undef protected

/*
 * set global environment
 */
class DeferEnvironment : public testing::Environment {
public:
    DeferEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

DeferEnvironment *env;

namespace defertest {
namespace {

/*
 * DeferTest, use googletest
 */
class DeferTest: public ::testing::Test {

protected:

    void LambdaTest1() {
        for (int i=0; i<3; i++) test_val[i] = 0;
        defer( [&] () { test_val[2] = 9;} );
        test_val[0] = 1;
        test_val[1] = 2;
    }

    void LambdaTest2() {
        for (int i=0; i<3; i++) test_val[i] = 0;
        defer( [&] () { test_val[2] = 9;} );
        test_val[0] = 1;
        return;
        test_val[1] = 2;
    }

    void LambdaTest3() {
        for (int i=0; i<3; i++) test_val[i] = 0;
        defer( [&] () { test_val[2] = 9;} );
        return;
        test_val[0] = 1;
        test_val[1] = 2;
    }

    void LambdaTest4() {
        for (int i=0; i<3; i++) test_val[i] = 0;
        defer( [&] () { if (test_val[2] == 9) test_val[2] = 8;} );
        test_val[2] = 9;
    }

    void TestFunc() {
        LambdaTest1();
        EXPECT_TRUE(test_val[0] == 1);
        EXPECT_TRUE(test_val[1] == 2);
        EXPECT_TRUE(test_val[2] == 9);

        LambdaTest2();
        EXPECT_TRUE(test_val[0] == 1);
        EXPECT_TRUE(test_val[1] == 0);
        EXPECT_TRUE(test_val[2] == 9);

        LambdaTest3();
        EXPECT_TRUE(test_val[0] == 0);
        EXPECT_TRUE(test_val[1] == 0);
        EXPECT_TRUE(test_val[2] == 9);

        LambdaTest4();
        EXPECT_TRUE(test_val[0] == 0);
        EXPECT_TRUE(test_val[1] == 0);
        EXPECT_TRUE(test_val[2] == 8);
    }

private:
    int test_val[3];
};

TEST_F(DeferTest, LambdaTest) { TestFunc(); }

}  // namespace
}  // namespace defertest

GTEST_API_ int main(int argc, char **argv) {
    env = new DeferEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


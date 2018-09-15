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
#include "arraylist.h"
#undef private
#undef protected

/*
 * set global environment
 */
class ArraylistEnvironment : public testing::Environment {
public:
    ArraylistEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

ArraylistEnvironment *env;

namespace arraylisttest {
namespace {

/*
 * ArraylistTest, use googletest
 */
class ArraylistTest: public ::testing::Test {

protected:

    void MapTest1() {
        utility::ArrayList<int> list(32);
        int32_t head_id = 0;
        for (int i=0; i<24; i++) {
            list.AllocHead(head_id) = i;
        }
        int32_t expect_val = 23;
        EXPECT_TRUE(head_id != 0);
        while(head_id) {
            EXPECT_TRUE(expect_val == list.GetValue(head_id));
            head_id = list.GetNextId(head_id);
            expect_val--;
        }
    }

    void MapTest2() {
        utility::ArrayList<int> list(32);
        int32_t head_id[4] = {0};
        for (int i=0; i<1024; i++) {
            list.AllocHead(head_id[0]) = i;
            list.AllocHead(head_id[1]) = i;
            list.AllocHead(head_id[2]) = i;
            list.AllocHead(head_id[3]) = i;
        }
        int expect_val[4] = {1023, 1023, 1023, 1023};
        for (int i=0; i<4; i++) {
            EXPECT_TRUE(head_id[i] != 0);
            while(head_id[i]) {
                EXPECT_TRUE(expect_val[i] == list.GetValue(head_id[i]));
                head_id[i] = list.GetNextId(head_id[i]);
                expect_val[i]--;
            }
        }
        list.DeleteLinks(head_id[0]);
        EXPECT_TRUE(head_id[0] == 0);

        for (int i=0; i<4; i++) {
            head_id[i] = 0;
            expect_val[i] = 1023;
        }
        for (int i=0; i<1024; i++) {
            list.AllocHead(head_id[0]) = i;
            list.AllocHead(head_id[1]) = i;
            list.AllocHead(head_id[2]) = i;
            list.AllocHead(head_id[3]) = i;
        }
        for (int i=0; i<4; i++) {
            EXPECT_TRUE(head_id[i] != 0);
            while(head_id[i]) {
                EXPECT_TRUE(expect_val[i] == list.GetValue(head_id[i]));
                head_id[i] = list.GetNextId(head_id[i]);
                expect_val[i]--;
            }
        }
    }

private:
};

TEST_F(ArraylistTest, MapTest1) { MapTest1(); }
TEST_F(ArraylistTest, MapTest2) { MapTest2(); }

}  // namespace
}  // namespace arraylisttest

GTEST_API_ int main(int argc, char **argv) {
    env = new ArraylistEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


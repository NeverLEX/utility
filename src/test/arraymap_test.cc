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
#include "arraymap.h"
#undef private
#undef protected

/*
 * set global environment
 */
class ArraymapEnvironment : public testing::Environment {
public:
    ArraymapEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

ArraymapEnvironment *env;

namespace arraymaptest {
namespace {

/*
 * ArraymapTest, use googletest
 */
class ArraymapTest: public ::testing::Test {

protected:

    void MapTest1() {
        utility::ArrayMap<int, int> arr_map(10124);
        for (int i=0; i<64; i++) {
            arr_map[i] = i;
        }
        for (int i=0; i<64; i++) {
            EXPECT_TRUE(arr_map.Find(i) == i);
        }
        arr_map.Delete(31);
        arr_map.Delete(11);
        EXPECT_TRUE(arr_map.Find(31) == 0);
        EXPECT_TRUE(arr_map.Find(11) == 0);
    }

    void MapTest2() {
        utility::ArrayMap<int, int> arr_map(12412);
        for (int i=0; i<1232; i++) {
            arr_map[i] = i;
        }
        for (int i=0; i<1232; i++) {
            EXPECT_TRUE(arr_map.Find(i) == i);
        }
        arr_map.Delete(31);
        arr_map.Delete(11);
        EXPECT_TRUE(arr_map.Find(31) == 0);
        EXPECT_TRUE(arr_map.Find(11) == 0);
        for (int i=111; i<1232; i++) {
            EXPECT_TRUE(arr_map.Find(i) == i);
        }
    }

    void MapTest3() {
        utility::ArrayMap<int, int> arr_map(2);
        for (int i=0; i<1232; i++) {
            arr_map[i] = i;
        }
        for (int i=0; i<1232; i++) {
            EXPECT_TRUE(arr_map.Find(i) == i);
        }
        arr_map.Delete(31);
        arr_map.Delete(11);
        EXPECT_TRUE(arr_map.Find(31) == 0);
        EXPECT_TRUE(arr_map.Find(11) == 0);
        for (int i=111; i<1232; i++) {
            EXPECT_TRUE(arr_map.Find(i) == i);
        }
    }

private:
};

TEST_F(ArraymapTest, MapTest1) { MapTest1(); }
TEST_F(ArraymapTest, MapTest2) { MapTest2(); }
TEST_F(ArraymapTest, MapTest3) { MapTest3(); }

}  // namespace
}  // namespace arraymaptest

GTEST_API_ int main(int argc, char **argv) {
    env = new ArraymapEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


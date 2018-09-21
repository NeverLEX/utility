#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
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
#include "arraypool.h"
#undef private
#undef protected

/*
 * set global environment
 */
class ArrayPoolEnvironment : public testing::Environment {
public:
    ArrayPoolEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

ArrayPoolEnvironment *env;

namespace arraypooltest {
namespace {

/*
 * ArrayPoolTest, use googletest
 */
class ArrayPoolTest: public ::testing::Test {

protected:

    void ArrayPoolTest1() {
        utility::ArrayPool<int> pool(32);
        std::vector<int*> alloc_save;
        alloc_save.resize(1021);
        for (int i=0; i<1021; i++) {
            alloc_save[i] = pool.Alloc();
            *alloc_save[i] = i;
        }
        for (int i=0; i<1021; i++) {
            EXPECT_TRUE(*alloc_save[i] == i);
        }
        for (int i=0; i<1021; i++) {
            pool.Delete(alloc_save[i]);
        }
        EXPECT_TRUE(pool.size() == 0);
    }

    void ArrayPoolTest2() {
        utility::ArrayPool<int> pool(32);
        std::vector<int*> alloc_save;
        alloc_save.resize(1021);
        for (int i=0; i<100; i++) {
            for (int i=0; i<1021; i++) {
                alloc_save[i] = pool.Alloc();
                *alloc_save[i] = i;
            }
            for (int i=0; i<1021; i++) {
                EXPECT_TRUE(*alloc_save[i] == i);
            }
            for (int i=0; i<100; i++) {
                pool.Delete(alloc_save[i]);
            }
            for (int i=0; i<100; i++) {
                alloc_save[i] = pool.Alloc();
                *alloc_save[i] = i;
            }
            for (int i=0; i<1021; i++) {
                EXPECT_TRUE(*alloc_save[i] == i);
            }
            for (int i=0; i<1021; i++) {
                pool.Delete(alloc_save[i]);
            }
        }
        EXPECT_TRUE(pool.size() == 0);
    }

    void ArrayPoolTest3() {
        utility::ArrayPool<int> pool(1<<16);
        std::vector<int*> alloc_save;
        alloc_save.resize(93497);
        for (int i=0; i<5; i++) {
            for (int i=0; i<93497; i++) {
                alloc_save[i] = pool.Alloc();
                *alloc_save[i] = i;
            }
            for (int i=0; i<93497; i++) {
                EXPECT_TRUE(*alloc_save[i] == i);
            }
            for (int i=0; i<30000; i++) {
                pool.Delete(alloc_save[i]);
            }
            for (int i=0; i<30000; i++) {
                alloc_save[i] = pool.Alloc();
                *alloc_save[i] = i;
            }
            for (int i=0; i<93497; i++) {
                EXPECT_TRUE(*alloc_save[i] == i);
            }
            for (int i=0; i<93497; i++) {
                pool.Delete(alloc_save[i]);
            }
        }
        EXPECT_TRUE(pool.size() == 0);
    }

private:
};

TEST_F(ArrayPoolTest, ArrayPoolTest1) { ArrayPoolTest1(); }
TEST_F(ArrayPoolTest, ArrayPoolTest2) { ArrayPoolTest2(); }
TEST_F(ArrayPoolTest, ArrayPoolTest3) { ArrayPoolTest3(); }

}  // namespace
}  // namespace arraypooltest

GTEST_API_ int main(int argc, char **argv) {
    env = new ArrayPoolEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


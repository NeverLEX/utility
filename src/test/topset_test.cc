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
#include "topset.h"
#undef private
#undef protected

/*
 * set global environment
 */
class TopSetEnvironment : public testing::Environment {
public:
    TopSetEnvironment() {}

protected:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

TopSetEnvironment *env;

namespace topsettest {
namespace {

/*
 * TopSetTest, use googletest
 */
class TopSetTest: public ::testing::Test {

protected:

    void TopSetTest1() {
        typedef utility::topset<int, int, 32> TopSetA;
        TopSetA top_set;
        for (int i=0; i<1024; i++) {
            top_set.Insert(i, i);
        }
        const TopSetA::Elem* p = top_set.Sort();
        EXPECT_TRUE(top_set.size() == 32);
        for (int i=0; i<32; i++, p++) {
            EXPECT_TRUE(p->val == i + 1024-32);
        }
    }

    void TopSetTest2() {
        typedef utility::topset<int, int, 32> TopSetA;
        TopSetA top_set;
        for (int i=0; i<24; i++) {
            top_set.Insert(i, i);
        }
        const TopSetA::Elem* p = top_set.Sort();
        EXPECT_TRUE(top_set.size() == 24);
        for (int i=0; i<24; i++, p++) {
            EXPECT_TRUE(p->val == i);
        }
    }

    void TopSetTest3() {
        typedef utility::topset<int, int, 31> TopSetA;
        TopSetA top_set;
        for (int i=0; i<1024; i++) {
            top_set.Insert(i, i);
        }
        const TopSetA::Elem* p = top_set.Sort();
        EXPECT_TRUE(top_set.size() == 31);
        for (int i=0; i<31; i++, p++) {
            EXPECT_TRUE(p->val == i + 1024-31);
        }
    }

private:
};

TEST_F(TopSetTest, TopSetTest1) { TopSetTest1(); }
TEST_F(TopSetTest, TopSetTest2) { TopSetTest2(); }
TEST_F(TopSetTest, TopSetTest3) { TopSetTest3(); }

}  // namespace
}  // namespace topsettest

GTEST_API_ int main(int argc, char **argv) {
    env = new TopSetEnvironment();
    testing::AddGlobalTestEnvironment(env);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


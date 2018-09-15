/*
 * simple implement of gtest, only support TEST_F, EXPECT_TRUE, EXPECT_FALSE
 */

#pragma once
#include <iostream>
#include <map>
#include <vector>

namespace testing {

// environment
class Environment {
public:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

class Test {
public:
    virtual void SetUp() {}

    virtual void TearDown() {}
};

// single test case
class TestCase : public Test {
public:
    TestCase(const char* name) :test_casename_(name) {};
    virtual ~TestCase() {}
    virtual void run() = 0;
    int test_result_;
    const char* test_casename_;
};

// test case manager
class TestCaseManager {
public:
    static TestCaseManager* getInstance() {
        static TestCaseManager _;
        return &_;
    };

    ~TestCaseManager() {
        for (std::vector<TestCase*>::iterator i = test_cases_.begin(); i != test_cases_.end(); ++i) {
            TestCase *case_tmp = *i;
            if (case_tmp) delete case_tmp;
        }
    }

    // run all test
    int run() {
        all_count_ = test_cases_.size();
        success_count_ = 0;
        env_->SetUp();
        std::cout << std::endl << "[-----------------RUN-----------------]" << std::endl;
        for (std::vector<TestCase*>::iterator i = test_cases_.begin(); i != test_cases_.end(); ++i) {
            current_test_case_ = *i;
            current_test_case_->test_result_ = 1;
            std::cout << "[RUN     ] " << current_test_case_->test_casename_ << std::endl;
            current_test_case_->SetUp();  // set up current test case env
            current_test_case_->run();  // run current test case
            current_test_case_->TearDown();  // tear down current test case
            if (current_test_case_->test_result_) success_count_++;
            std::cout << "[     END] " << current_test_case_->test_casename_ << std::endl;

        }
        std::cout << "[-----------------END-----------------]" << std::endl;
        std::cout << "[TOTAL   ] " << all_count_ << std::endl;
        std::cout << "[SUCCESS ] " << success_count_ << std::endl;
        std::cout << "[FAIL    ] " << all_count_ - success_count_ << std::endl;
        env_->TearDown();
        return all_count_ - success_count_;
    };

    // register test case
    TestCase* registerTestCase(TestCase* one) {
        test_cases_.push_back(one);
        return one;
    }

    TestCase* GetCurrentCase() { return current_test_case_; }

    void SetEnvironment(Environment *env) { env_ = env; }

private:
    std::vector<TestCase*> test_cases_;
    TestCase* current_test_case_;
    Environment* env_;
    int success_count_;
    int all_count_;
};

// do nothing
void AddGlobalTestEnvironment(Environment *env) {
    testing::TestCaseManager::getInstance()->SetEnvironment(env);
}

// do nothing
void InitGoogleTest(int *argc, char *argv[]) {

}

}  // namespace testing

#define CTEST_TO_STRING(name) _CTEST_TO_STRING(name)
#define _CTEST_TO_STRING(name) #name
#define TESTCASE_NAME(testcase_name, test_name) testcase_name##_##test_name##_Test

#define CTEST_(testcase_name, test_name) \
class TESTCASE_NAME(testcase_name, test_name) : public testing::TestCase , public testcase_name { \
public: \
    TESTCASE_NAME(testcase_name, test_name)(const char* casename) : testing::TestCase(casename){};   \
    virtual void run(); \
private: \
    static testing::TestCase * const _testcase; \
}; \
testing::TestCase* const TESTCASE_NAME(testcase_name, test_name) \
::_testcase = testing::TestCaseManager::getInstance()->registerTestCase(new TESTCASE_NAME(testcase_name, test_name)(CTEST_TO_STRING(TESTCASE_NAME(testcase_name, test_name)))); \
void TESTCASE_NAME(testcase_name, test_name)::run()

#define TEST_F(testcase_name, test_name) \
    CTEST_(testcase_name, test_name)

#define GTEST_API_

#define RUN_ALL_TESTS()   \
    testing::TestCaseManager::getInstance()->run()

#define EXPECT_TRUE(exp) \
    if (!(exp)) { \
        testing::TestCaseManager::getInstance()->GetCurrentCase()->test_result_   = 0; \
        std::cout << "fail to check: "#exp << std::endl; \
        std::cout << "expected: true, actual: false" << std::endl; \
        std::cout << "location: " << __FILE__ << " : " << __LINE__ << std::endl; \
    } \

#define EXPECT_FALSE(exp) \
    if((exp)) { \
        testing::TestCaseManager::getInstance()->GetCurrentCase()->test_result_   = 0; \
        std::cout << "fail to check: "#exp << std::endl; \
        std::cout << "expected: false, actual: true" << std::endl; \
        std::cout << "location: " << __FILE__ << " : " << __LINE__ << std::endl; \
    } \


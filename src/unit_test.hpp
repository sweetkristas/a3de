#pragma once

#include <boost/function.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace test {

	struct failure_exception 
	{
	};

	typedef boost::function<void ()> unit_test;

	int register_test(const std::string& name, unit_test test);
	
	bool run_tests(const std::vector<std::string>* tests=NULL);
}

#define CHECK(cond, msg) if(!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << ": TEST CHECK FAILED: " << #cond << ": " << msg << "\n"; throw test::failure_exception(); }

#define CHECK_CMP(a, b, cmp) CHECK((a) cmp (b), #a << ": " << (a) << "; " << #b << ": " << (b))

#define CHECK_EQ(a, b) CHECK_CMP(a, b, ==)
#define CHECK_NE(a, b) CHECK_CMP(a, b, !=)
#define CHECK_LE(a, b) CHECK_CMP(a, b, <=)
#define CHECK_GE(a, b) CHECK_CMP(a, b, >=)
#define CHECK_LT(a, b) CHECK_CMP(a, b, <)
#define CHECK_GT(a, b) CHECK_CMP(a, b, >)

#define UNIT_TEST(name) \
	namespace test {    \
	void TEST_##name(); \
	static int TEST_VAR_##name = test::register_test(#name, TEST_##name); \
    }                   \
	void test::TEST_##name()
